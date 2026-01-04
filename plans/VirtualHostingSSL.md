# Virtual Hosting SSL/TLS Implementation Plan

## Overview

Extend NeonSignal's virtual hosting to support **per-domain TLS certificates** using **SNI (Server Name Indication)**. This allows serving different SSL certificates based on the hostname requested during the TLS handshake.

```
Client requests "simonedelpopolo.host"     → Uses certs/simonedelpopolo.host/
Client requests "neonsignal.nutsloop.host" → Uses certs/neonsignal.nutsloop.host/
Client requests unknown domain              → Uses certs/_default/ (fallback)
```

---

## How SNI Works

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         TLS Handshake with SNI                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  Client                                              Server             │
│    │                                                    │               │
│    │─── ClientHello ──────────────────────────────────►│               │
│    │    (includes SNI extension:                        │               │
│    │     server_name = "neonsignal.nutsloop.host")      │               │
│    │                                                    │               │
│    │                    ┌───────────────────────────────┤               │
│    │                    │ SNI Callback triggered:       │               │
│    │                    │ 1. Extract hostname           │               │
│    │                    │ 2. Look up SSL_CTX for host   │               │
│    │                    │ 3. Switch to that SSL_CTX     │               │
│    │                    └───────────────────────────────┤               │
│    │                                                    │               │
│    │◄── ServerHello + Certificate ─────────────────────│               │
│    │    (uses cert for neonsignal.nutsloop.host)       │               │
│    │                                                    │               │
│    │─── Key Exchange, Finished ───────────────────────►│               │
│    │◄── Finished ──────────────────────────────────────│               │
│    │                                                    │               │
│    │════════ Encrypted Application Data ═══════════════│               │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

**Key Point:** SNI happens BEFORE HTTP. The server must decide which certificate to use based only on the hostname in the TLS ClientHello, not the HTTP `:authority` header.

---

## Design Decisions

### 1. Certificate Directory Structure

**Option A: Flat domain directories** (Recommended - matches vhost structure)
```
certs/
├── simonedelpopolo.host/
│   ├── fullchain.pem      # Certificate chain (cert + intermediates)
│   └── privkey.pem        # Private key
├── neonsignal.nutsloop.host/
│   ├── fullchain.pem
│   └── privkey.pem
├── *.nutsloop.host/       # Wildcard certificate (optional)
│   ├── fullchain.pem
│   └── privkey.pem
└── _default/              # Fallback certificate
    ├── fullchain.pem
    └── privkey.pem
```

**Option B: Single directory with naming convention**
```
certs/
├── simonedelpopolo.host.crt
├── simonedelpopolo.host.key
├── neonsignal.nutsloop.host.crt
├── neonsignal.nutsloop.host.key
└── _default.crt
└── _default.key
```

**Decision:** Option A - mirrors `public/` vhost structure, cleaner organization, supports additional cert files (CA bundles, OCSP stapling).

### 2. SSL Context Strategy

**Option A: Single SSL_CTX with SNI callback** (Simpler)
- One SSL_CTX handles all connections
- SNI callback dynamically loads/switches certificates
- Pros: Less memory, simpler lifecycle
- Cons: Certificate switching per-connection adds latency

**Option B: Multiple SSL_CTX pool** (Recommended - more performant)
- Pre-create one SSL_CTX per domain at startup
- SNI callback selects from pool
- Pros: Faster handshakes, certs pre-validated
- Cons: More memory, reload requires restart (or hot-reload logic)

**Decision:** Option B - pre-load all certificates at startup for performance.

### 3. Certificate Discovery

**Auto-discovery** (matches vhost approach):
- Scan `certs/` for directories that match domain patterns
- Each directory must contain `fullchain.pem` and `privkey.pem`
- Validate certificates are readable and not expired at startup

---

## Implementation Plan

### Phase 1: Certificate Manager

#### 1.1 Create CertManager Class

**File:** `include/neonsignal/cert_manager.h++`

```cpp
#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <openssl/ssl.h>

namespace neonsignal {

struct CertificateBundle {
  std::string domain;
  std::filesystem::path cert_path;   // fullchain.pem
  std::filesystem::path key_path;    // privkey.pem
  std::unique_ptr<SSL_CTX, void(*)(SSL_CTX*)> ssl_ctx{nullptr, SSL_CTX_free};
  bool is_wildcard{false};

  // Certificate metadata (populated during load)
  std::string common_name;
  std::vector<std::string> san_names;  // Subject Alternative Names
  std::time_t not_before{0};
  std::time_t not_after{0};
};

class CertManager {
public:
  explicit CertManager(std::filesystem::path certs_root);

  // Initialize: scan certs directory, load all certificates
  // Returns false if _default cert is missing or invalid
  bool initialize();

  // Get SSL_CTX for a domain (called from SNI callback)
  // Returns nullptr if no matching cert found
  SSL_CTX* get_context(std::string_view hostname) const;

  // Get default SSL_CTX (for non-SNI clients or unknown domains)
  SSL_CTX* get_default_context() const;

  // Reload certificates (for hot-reload support)
  bool reload();

  // List loaded certificates for logging
  std::vector<std::string> list_certificates() const;

  // Check if any certificates expire within N days
  std::vector<std::string> expiring_soon(int days = 30) const;

private:
  std::filesystem::path certs_root_;
  mutable std::shared_mutex mutex_;

  std::unordered_map<std::string, std::unique_ptr<CertificateBundle>> exact_certs_;
  std::vector<std::unique_ptr<CertificateBundle>> wildcard_certs_;
  CertificateBundle* default_cert_{nullptr};

  // Load a single certificate bundle
  std::unique_ptr<CertificateBundle> load_certificate(
      const std::filesystem::path& cert_dir,
      const std::string& domain);

  // Create and configure SSL_CTX for a certificate
  bool configure_ssl_ctx(CertificateBundle& bundle);

  // Extract certificate metadata (CN, SANs, validity)
  bool extract_cert_info(CertificateBundle& bundle);

  // Normalize hostname for lookup
  static std::string normalize_hostname(std::string_view hostname);

  // Check if directory looks like a domain cert directory
  static bool is_cert_directory(const std::filesystem::path& path);
};

} // namespace neonsignal
```

#### 1.2 Implement CertManager

**File:** `src/neonsignal/cert_manager/initialize.c++`

```cpp
#include <neonsignal/cert_manager.h++>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <iostream>
#include <algorithm>

namespace neonsignal {

CertManager::CertManager(std::filesystem::path certs_root)
    : certs_root_(std::move(certs_root)) {}

bool CertManager::initialize() {
  std::unique_lock lock(mutex_);
  exact_certs_.clear();
  wildcard_certs_.clear();
  default_cert_ = nullptr;

  if (!std::filesystem::is_directory(certs_root_)) {
    std::cerr << "CertManager: certs directory not found: "
              << certs_root_ << '\n';
    return false;
  }

  for (const auto& entry : std::filesystem::directory_iterator(certs_root_)) {
    if (!entry.is_directory()) continue;
    if (!is_cert_directory(entry.path())) continue;

    std::string name = entry.path().filename().string();
    auto bundle = load_certificate(entry.path(), name);

    if (!bundle) {
      std::cerr << "CertManager: failed to load cert for " << name << '\n';
      continue;
    }

    if (name == "_default") {
      default_cert_ = bundle.get();
      exact_certs_["_default"] = std::move(bundle);
    } else if (name.starts_with("*.")) {
      bundle->is_wildcard = true;
      bundle->domain = name.substr(2);  // Remove "*."
      wildcard_certs_.push_back(std::move(bundle));
    } else {
      std::string normalized = normalize_hostname(name);
      exact_certs_[normalized] = std::move(bundle);
    }
  }

  if (!default_cert_) {
    std::cerr << "CertManager: WARNING - no _default certificate found!\n";
    // Try to use first available cert as fallback
    if (!exact_certs_.empty()) {
      for (auto& [name, bundle] : exact_certs_) {
        if (name != "_default") {
          default_cert_ = bundle.get();
          std::cerr << "CertManager: using " << name << " as fallback\n";
          break;
        }
      }
    }
  }

  return default_cert_ != nullptr;
}

bool CertManager::is_cert_directory(const std::filesystem::path& path) {
  // Must contain fullchain.pem and privkey.pem
  return std::filesystem::exists(path / "fullchain.pem") &&
         std::filesystem::exists(path / "privkey.pem");
}

std::unique_ptr<CertificateBundle> CertManager::load_certificate(
    const std::filesystem::path& cert_dir,
    const std::string& domain) {

  auto bundle = std::make_unique<CertificateBundle>();
  bundle->domain = domain;
  bundle->cert_path = cert_dir / "fullchain.pem";
  bundle->key_path = cert_dir / "privkey.pem";

  if (!configure_ssl_ctx(*bundle)) {
    return nullptr;
  }

  if (!extract_cert_info(*bundle)) {
    std::cerr << "CertManager: warning - could not extract cert info for "
              << domain << '\n';
  }

  return bundle;
}

bool CertManager::configure_ssl_ctx(CertificateBundle& bundle) {
  SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    return false;
  }

  bundle.ssl_ctx.reset(ctx);

  // Set minimum TLS version to 1.2
  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

  // Load certificate chain
  if (SSL_CTX_use_certificate_chain_file(ctx, bundle.cert_path.c_str()) != 1) {
    std::cerr << "CertManager: failed to load certificate: "
              << bundle.cert_path << '\n';
    ERR_print_errors_fp(stderr);
    return false;
  }

  // Load private key
  if (SSL_CTX_use_PrivateKey_file(ctx, bundle.key_path.c_str(),
                                   SSL_FILETYPE_PEM) != 1) {
    std::cerr << "CertManager: failed to load private key: "
              << bundle.key_path << '\n';
    ERR_print_errors_fp(stderr);
    return false;
  }

  // Verify key matches certificate
  if (SSL_CTX_check_private_key(ctx) != 1) {
    std::cerr << "CertManager: private key does not match certificate for "
              << bundle.domain << '\n';
    return false;
  }

  // Configure ALPN for HTTP/2
  static const unsigned char alpn[] = {2, 'h', '2'};
  SSL_CTX_set_alpn_select_cb(ctx, [](SSL*, const unsigned char** out,
                                      unsigned char* outlen,
                                      const unsigned char* in,
                                      unsigned int inlen, void*) -> int {
    if (SSL_select_next_proto(const_cast<unsigned char**>(out), outlen,
                              alpn, sizeof(alpn), in, inlen) == OPENSSL_NPN_NEGOTIATED) {
      return SSL_TLSEXT_ERR_OK;
    }
    return SSL_TLSEXT_ERR_NOACK;
  }, nullptr);

  std::cerr << "CertManager: loaded certificate for " << bundle.domain << '\n';
  return true;
}

} // namespace neonsignal
```

**File:** `src/neonsignal/cert_manager/get_context.c++`

```cpp
#include <neonsignal/cert_manager.h++>
#include <algorithm>

namespace neonsignal {

std::string CertManager::normalize_hostname(std::string_view hostname) {
  // Strip port if present
  auto pos = hostname.find(':');
  if (pos != std::string_view::npos) {
    hostname = hostname.substr(0, pos);
  }

  // Convert to lowercase
  std::string result(hostname);
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

SSL_CTX* CertManager::get_context(std::string_view hostname) const {
  std::shared_lock lock(mutex_);

  std::string normalized = normalize_hostname(hostname);

  // 1. Exact match
  if (auto it = exact_certs_.find(normalized); it != exact_certs_.end()) {
    return it->second->ssl_ctx.get();
  }

  // 2. Wildcard match (*.example.com matches sub.example.com)
  for (const auto& wc : wildcard_certs_) {
    if (normalized.size() > wc->domain.size() + 1) {
      auto suffix_start = normalized.size() - wc->domain.size();
      if (normalized[suffix_start - 1] == '.' &&
          normalized.substr(suffix_start) == wc->domain) {
        return wc->ssl_ctx.get();
      }
    }
  }

  // 3. Check if hostname is covered by any cert's SAN
  for (const auto& [name, bundle] : exact_certs_) {
    for (const auto& san : bundle->san_names) {
      if (san == normalized) {
        return bundle->ssl_ctx.get();
      }
      // Wildcard SAN match
      if (san.starts_with("*.")) {
        std::string_view san_domain = std::string_view(san).substr(2);
        if (normalized.size() > san_domain.size() + 1) {
          auto suffix_start = normalized.size() - san_domain.size();
          if (normalized[suffix_start - 1] == '.' &&
              normalized.substr(suffix_start) == san_domain) {
            return bundle->ssl_ctx.get();
          }
        }
      }
    }
  }

  // 4. Return default
  return default_cert_ ? default_cert_->ssl_ctx.get() : nullptr;
}

SSL_CTX* CertManager::get_default_context() const {
  std::shared_lock lock(mutex_);
  return default_cert_ ? default_cert_->ssl_ctx.get() : nullptr;
}

} // namespace neonsignal
```

### Phase 2: SNI Callback Integration

#### 2.1 Add SNI Callback to Server

**File:** `src/neonsignal/server/initialize_tls.c++` (modified)

```cpp
#include <neonsignal/server.h++>
#include <neonsignal/cert_manager.h++>
#include <openssl/ssl.h>
#include <iostream>

namespace neonsignal {

// SNI callback - called during TLS handshake
static int sni_callback(SSL* ssl, int* alert, void* arg) {
  CertManager* cert_mgr = static_cast<CertManager*>(arg);

  const char* servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  if (!servername) {
    // No SNI extension - use default cert
    return SSL_TLSEXT_ERR_OK;
  }

  SSL_CTX* ctx = cert_mgr->get_context(servername);
  if (!ctx) {
    // No matching cert - use current (default) context
    std::cerr << "SNI: no cert for " << servername << ", using default\n";
    return SSL_TLSEXT_ERR_OK;
  }

  // Switch to the domain-specific SSL_CTX
  SSL_set_SSL_CTX(ssl, ctx);

  return SSL_TLSEXT_ERR_OK;
}

void Server::initialize_tls() {
  // Initialize CertManager
  cert_manager_ = std::make_unique<CertManager>(config_.certs_root);
  if (!cert_manager_->initialize()) {
    throw std::runtime_error("Failed to initialize certificate manager");
  }

  // Get default context for the main SSL_CTX
  ssl_ctx_.reset(cert_manager_->get_default_context());

  // But we need to keep our own reference, so create a wrapper
  SSL_CTX* default_ctx = cert_manager_->get_default_context();
  if (!default_ctx) {
    throw std::runtime_error("No default certificate available");
  }

  // Register SNI callback on all contexts
  for (const auto& cert : cert_manager_->list_certificates()) {
    // The callback is registered per-context
  }

  // Actually, we register SNI callback on the default context
  // When a new SSL object is created from this context, it inherits the callback
  SSL_CTX_set_tlsext_servername_callback(default_ctx, sni_callback);
  SSL_CTX_set_tlsext_servername_arg(default_ctx, cert_manager_.get());

  // Log loaded certificates
  std::cerr << "TLS initialized with certificates:\n";
  for (const auto& cert : cert_manager_->list_certificates()) {
    std::cerr << "  " << cert << '\n';
  }

  // Warn about expiring certificates
  auto expiring = cert_manager_->expiring_soon(30);
  for (const auto& cert : expiring) {
    std::cerr << "  WARNING: " << cert << " expires within 30 days!\n";
  }
}

} // namespace neonsignal
```

### Phase 3: Configuration Updates

#### 3.1 Update ServerConfig

**File:** `include/neonsignal/neonsignal.h++` (modified)

```cpp
struct ServerConfig {
  std::string host = "0.0.0.0";
  std::uint16_t port = 9443;

  // Old single-cert config (deprecated, kept for backward compatibility)
  std::string cert_path = "certs/server.crt";
  std::string key_path = "certs/server.key";

  // New multi-cert config
  std::string certs_root = "certs";  // Directory containing per-domain cert folders

  std::string public_root = "public";
  std::string rp_id = "simonedelpopolo.host";
  std::string origin = "https://simonedelpopolo.host";
  std::string credentials_path = "config/credentials.json";
};
```

---

## Certificate Directory Setup

### Initial Migration

```bash
# 1. Create new directory structure
mkdir -p certs/_default
mkdir -p certs/simonedelpopolo.host
mkdir -p certs/neonsignal.nutsloop.host

# 2. Move existing default cert
mv certs/server.crt certs/_default/fullchain.pem
mv certs/server.key certs/_default/privkey.pem

# 3. If using Let's Encrypt, symlink or copy
# For simonedelpopolo.host:
ln -s /etc/letsencrypt/live/simonedelpopolo.host/fullchain.pem certs/simonedelpopolo.host/
ln -s /etc/letsencrypt/live/simonedelpopolo.host/privkey.pem certs/simonedelpopolo.host/

# For neonsignal.nutsloop.host:
ln -s /etc/letsencrypt/live/neonsignal.nutsloop.host/fullchain.pem certs/neonsignal.nutsloop.host/
ln -s /etc/letsencrypt/live/neonsignal.nutsloop.host/privkey.pem certs/neonsignal.nutsloop.host/
```

### Using Wildcard Certificate

If you have a wildcard certificate for `*.nutsloop.host`:

```bash
mkdir -p "certs/*.nutsloop.host"
ln -s /etc/letsencrypt/live/nutsloop.host/fullchain.pem "certs/*.nutsloop.host/"
ln -s /etc/letsencrypt/live/nutsloop.host/privkey.pem "certs/*.nutsloop.host/"
```

This will match:
- `neonsignal.nutsloop.host`
- `api.nutsloop.host`
- `anything.nutsloop.host`

---

## Startup Logging

```
neonsignal->TLS initialized with certificates:
  _default -> certs/_default/ (CN=localhost, expires 2025-12-31)
  neonsignal.nutsloop.host -> certs/neonsignal.nutsloop.host/ (CN=neonsignal.nutsloop.host, expires 2026-03-15)
  simonedelpopolo.host -> certs/simonedelpopolo.host/ (CN=simonedelpopolo.host, expires 2026-03-15)
  *.nutsloop.host -> certs/*.nutsloop.host/ (CN=*.nutsloop.host, expires 2026-03-15)
```

---

## Files to Create/Modify

### New Files

| File | Purpose |
|------|---------|
| `include/neonsignal/cert_manager.h++` | CertManager class declaration |
| `src/neonsignal/cert_manager.c++` | Constructor |
| `src/neonsignal/cert_manager/initialize.c++` | Certificate loading and validation |
| `src/neonsignal/cert_manager/get_context.c++` | SNI lookup logic |
| `src/neonsignal/cert_manager/reload.c++` | Hot-reload support |
| `src/neonsignal/cert_manager/expiry_check.c++` | Certificate expiration warnings |

### Modified Files

| File | Changes |
|------|---------|
| `include/neonsignal/neonsignal.h++` | Add `certs_root` to ServerConfig |
| `include/neonsignal/server.h++` | Add CertManager member |
| `src/neonsignal/server/initialize_tls.c++` | Use CertManager, set up SNI callback |
| `src/neonsignal/server/run.c++` | Add NEONSIGNAL_CERTS_ROOT env var support |
| `src/meson.build` | Add cert_manager sources |

---

## Testing Plan

### Manual Testing

```bash
# Test SNI with curl
curl -v --resolve simonedelpopolo.host:443:127.0.0.1 https://simonedelpopolo.host/

curl -v --resolve neonsignal.nutsloop.host:443:127.0.0.1 https://neonsignal.nutsloop.host/

# Check which certificate is returned
openssl s_client -connect 127.0.0.1:443 -servername simonedelpopolo.host </dev/null 2>/dev/null | openssl x509 -noout -subject

openssl s_client -connect 127.0.0.1:443 -servername neonsignal.nutsloop.host </dev/null 2>/dev/null | openssl x509 -noout -subject

# Test without SNI (should get default cert)
openssl s_client -connect 127.0.0.1:443 -noservername </dev/null 2>/dev/null | openssl x509 -noout -subject
```

### Automated Tests

1. SNI callback returns correct SSL_CTX for each domain
2. Wildcard matching works correctly
3. Unknown domains fall back to default
4. Certificate expiration warnings are logged
5. Invalid certificates are rejected at startup

---

## Future Enhancements

1. **OCSP Stapling** - Embed revocation status in TLS handshake
2. **Certificate hot-reload** - Reload certs without restart (via SIGHUP)
3. **Automatic renewal** - Integrate with certbot hooks
4. **ACME integration** - Built-in Let's Encrypt support
5. **Certificate transparency logging** - Submit to CT logs

---

## Security Considerations

1. **Private key permissions** - Ensure `privkey.pem` files are readable only by the neonsignal process (mode 0600)
2. **Certificate validation** - Verify cert chain completeness at startup
3. **Weak cipher rejection** - Configure strong cipher suites only
4. **HSTS headers** - Consider adding Strict-Transport-Security per-vhost

---

## Implementation Order

1. **Create `cert_manager.h++`** - Class declaration
2. **Implement `initialize.c++`** - Load certificates at startup
3. **Implement `get_context.c++`** - SNI lookup logic
4. **Modify `initialize_tls.c++`** - Register SNI callback
5. **Update `meson.build`** - Add new source files
6. **Create cert directory structure** - Migrate existing certs
7. **Test** - Verify with openssl s_client and curl
8. **Add expiry checking** - Log warnings for soon-to-expire certs

**Estimated effort:** 4-6 hours for core implementation + testing
