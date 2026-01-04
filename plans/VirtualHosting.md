# Virtual Hosting Implementation Plan

## Overview

Implement **name-based virtual hosting** for NeonSignal, allowing a single server instance to serve different content based on the HTTP/2 `:authority` pseudo-header (equivalent to HTTP/1.1 `Host` header).

```
Request for simonedelpopolo.host     → public/simonedelpopolo.host/
Request for api.simonedelpopolo.host → public/api.simonedelpopolo.host/
Request for other.example.com        → public/other.example.com/
```

---

## Current Architecture

### What Exists

| Component | Status | Location |
|-----------|--------|----------|
| `:authority` header extraction | ✅ Available | `handle_io_.c++:207-220` |
| Static file routing | Single `public/` dir | `router/resolve.c++` |
| Configuration | Env vars + struct | `neonsignal.h++:17-26` |
| TLS certificates | Single cert/key | `server/configure_credentials.c++` |

### Key Code Points

```cpp
// handle_io_.c++:220 - Authority is already extracted
authority = parsed->authority;  // e.g., "simonedelpopolo.host"

// router/resolve.c++:23 - Currently uses single public_root
std::filesystem::path full = public_root_ / clean.substr(1);
```

---

## Design Decisions

### 1. Directory Structure

**Option A: Flat domain directories** (Recommended)
```
public/
├── simonedelpopolo.host/
│   ├── index.html
│   ├── app.js
│   └── css/
├── api.simonedelpopolo.host/
│   └── index.html
└── _default/                    # Fallback for unknown domains
    └── index.html
```

**Option B: Hierarchical subdomain structure**
```
public/
└── simonedelpopolo.host/
    ├── @/                       # Root domain content
    ├── api/                     # api.simonedelpopolo.host
    └── www/                     # www.simonedelpopolo.host
```

**Decision:** Option A - simpler, explicit, no ambiguity between paths and subdomains.

### 2. Domain Matching Strategy

```
Priority order:
1. Exact match:     "api.simonedelpopolo.host" → public/api.simonedelpopolo.host/
2. Wildcard match:  "*.simonedelpopolo.host"   → public/*.simonedelpopolo.host/
3. Default:         "_default"                  → public/_default/
4. Legacy fallback: (no vhost dir found)       → public/ (current behavior)
```

### 3. Configuration Approach

**Option A: Auto-discovery** (Recommended for simplicity)
- Scan `public/` for directories that look like domain names
- No config file needed
- Domain is valid if directory exists

**Option B: Explicit config file**
```json
{
  "vhosts": {
    "simonedelpopolo.host": {
      "root": "public/simonedelpopolo.host",
      "aliases": ["www.simonedelpopolo.host"]
    }
  }
}
```

**Decision:** Start with Option A (auto-discovery), add Option B later if needed.

---

## Implementation Plan

### Phase 1: Core Virtual Host Resolution

#### 1.1 Create VirtualHost Types

**File:** `include/neonsignal/vhost.h++`

```cpp
#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <shared_mutex>

namespace neonsignal {

struct VirtualHost {
    std::string domain;
    std::filesystem::path document_root;
    bool is_wildcard{false};
};

class VHostResolver {
public:
    explicit VHostResolver(std::filesystem::path public_root);

    // Resolve domain to document root
    // Returns nullopt if no vhost found (use legacy behavior)
    std::optional<std::filesystem::path> resolve(std::string_view authority) const;

    // Rescan public directory for vhost directories
    void refresh();

    // Check if vhosting is enabled (any vhost dirs exist)
    bool enabled() const;

private:
    std::filesystem::path public_root_;
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, VirtualHost> exact_hosts_;
    std::vector<VirtualHost> wildcard_hosts_;  // For *.domain.com patterns
    bool has_default_{false};

    // Extract domain from authority (strips port if present)
    static std::string normalize_authority(std::string_view authority);

    // Check if directory name looks like a domain
    static bool is_domain_directory(std::string_view name);
};

} // namespace neonsignal
```

#### 1.2 Implement VHostResolver

**File:** `src/neonsignal/vhost/resolve.c++`

```cpp
#include <neonsignal/vhost.h++>
#include <regex>
#include <algorithm>

namespace neonsignal {

VHostResolver::VHostResolver(std::filesystem::path public_root)
    : public_root_(std::move(public_root)) {
    refresh();
}

std::string VHostResolver::normalize_authority(std::string_view authority) {
    // Strip port if present: "example.com:443" → "example.com"
    auto pos = authority.find(':');
    if (pos != std::string_view::npos) {
        authority = authority.substr(0, pos);
    }
    // Convert to lowercase for case-insensitive matching
    std::string result(authority);
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool VHostResolver::is_domain_directory(std::string_view name) {
    // Match: domain.tld, sub.domain.tld, *.domain.tld, _default
    if (name == "_default") return true;
    if (name.starts_with('.') || name.ends_with('.')) return false;

    // Simple heuristic: contains at least one dot, alphanumeric + dots + hyphens + wildcard
    static const std::regex domain_pattern(
        R"(^(\*\.)?[a-z0-9]([a-z0-9\-]*[a-z0-9])?(\.[a-z0-9]([a-z0-9\-]*[a-z0-9])?)+$)",
        std::regex::icase
    );
    return std::regex_match(name.begin(), name.end(), domain_pattern);
}

void VHostResolver::refresh() {
    std::unique_lock lock(mutex_);
    exact_hosts_.clear();
    wildcard_hosts_.clear();
    has_default_ = false;

    if (!std::filesystem::is_directory(public_root_)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(public_root_)) {
        if (!entry.is_directory()) continue;

        std::string name = entry.path().filename().string();
        if (!is_domain_directory(name)) continue;

        VirtualHost vhost;
        vhost.domain = name;
        vhost.document_root = entry.path();

        if (name == "_default") {
            has_default_ = true;
            exact_hosts_["_default"] = std::move(vhost);
        } else if (name.starts_with("*.")) {
            vhost.is_wildcard = true;
            vhost.domain = name.substr(2);  // Remove "*."
            wildcard_hosts_.push_back(std::move(vhost));
        } else {
            exact_hosts_[name] = std::move(vhost);
        }
    }
}

std::optional<std::filesystem::path> VHostResolver::resolve(std::string_view authority) const {
    std::shared_lock lock(mutex_);

    if (exact_hosts_.empty() && wildcard_hosts_.empty()) {
        return std::nullopt;  // No vhosting configured
    }

    std::string domain = normalize_authority(authority);

    // 1. Exact match
    if (auto it = exact_hosts_.find(domain); it != exact_hosts_.end()) {
        return it->second.document_root;
    }

    // 2. Wildcard match (*.example.com matches sub.example.com)
    for (const auto& wc : wildcard_hosts_) {
        if (domain.size() > wc.domain.size() + 1) {
            // Check if domain ends with .wildcard_domain
            auto suffix_start = domain.size() - wc.domain.size();
            if (domain[suffix_start - 1] == '.' &&
                domain.substr(suffix_start) == wc.domain) {
                return wc.document_root;
            }
        }
    }

    // 3. Default fallback
    if (has_default_) {
        return exact_hosts_.at("_default").document_root;
    }

    return std::nullopt;
}

bool VHostResolver::enabled() const {
    std::shared_lock lock(mutex_);
    return !exact_hosts_.empty() || !wildcard_hosts_.empty();
}

} // namespace neonsignal
```

### Phase 2: Integrate with Router

#### 2.1 Modify Router to Accept Document Root Override

**File:** `include/neonsignal/router.h++`

```cpp
// Add new resolve overload
RouteResult resolve(std::string_view path,
                    const std::filesystem::path& document_root) const;
```

**File:** `src/neonsignal/router/resolve.c++`

```cpp
// Add overload that accepts custom document root
RouteResult Router::resolve(std::string_view path,
                           const std::filesystem::path& document_root) const {
    // Same logic as existing resolve(), but use document_root instead of public_root_
    RouteResult res;
    std::string clean(path);

    if (clean == "/") {
        clean = "/index.html";
    }
    if (clean.find("..") != std::string::npos) {
        return res;
    }

    std::filesystem::path full = document_root / clean.substr(1);

    if (std::filesystem::is_directory(full)) {
        full /= "index.html";
    }

    if (std::filesystem::is_regular_file(full)) {
        res.full_path = full.string();
        res.found = true;
    }
    return res;
}
```

### Phase 3: Integrate with Request Handler

#### 3.1 Add VHostResolver to Http2Listener

**File:** `include/neonsignal/http2_listener.h++`

```cpp
#include <neonsignal/vhost.h++>

class Http2Listener {
    // ... existing members ...
    VHostResolver vhost_resolver_;
};
```

#### 3.2 Modify Static File Loading

**File:** `src/neonsignal/http2_listener/handle_io_.c++`

After line 220 (where authority is extracted), add vhost resolution:

```cpp
// Resolve virtual host document root
std::filesystem::path doc_root = config_.public_root;
if (auto vhost_root = vhost_resolver_.resolve(authority)) {
    doc_root = *vhost_root;
}

// Pass doc_root to static file loading
auto loaded = load_static(path, doc_root, cache_, router_);
```

#### 3.3 Modify load_static Helper

**File:** `src/neonsignal/http2_listener/helper/load_static.c++`

```cpp
LoadedFile load_static(
    std::string_view path,
    const std::filesystem::path& document_root,  // NEW parameter
    StaticFileCache& cache,
    const Router& router
) {
    // Use document_root for resolution instead of global public_root
    auto route = router.resolve(path, document_root);
    // ... rest of implementation
}
```

### Phase 4: Startup Logging

#### 4.1 Log Discovered Virtual Hosts

**File:** `src/neonsignal/http2_listener/start.c++`

```cpp
void Http2Listener::start() {
    // Existing initialization...

    // Log virtual hosts
    if (vhost_resolver_.enabled()) {
        std::cerr << "neonsignal->Virtual hosts enabled:\n";
        // Log each discovered vhost
    } else {
        std::cerr << "neonsignal->No virtual hosts, using legacy single-root mode\n";
    }
}
```

---

## Migration Strategy

### Backward Compatibility

The implementation maintains full backward compatibility:

1. **No vhost directories exist:** Server uses `public/` directly (current behavior)
2. **Mixed mode:** If vhost directories exist alongside files in `public/`, vhosting takes priority for matching domains
3. **Gradual migration:** Move existing files into `public/simonedelpopolo.host/` when ready

### Migration Steps

```bash
# 1. Create vhost directory structure
mkdir -p public/simonedelpopolo.host

# 2. Move existing files (when ready)
mv public/*.html public/simonedelpopolo.host/
mv public/*.js public/simonedelpopolo.host/
mv public/css public/simonedelpopolo.host/
mv public/icons public/simonedelpopolo.host/

# 3. Create default fallback (optional)
mkdir -p public/_default
echo "<h1>Unknown Domain</h1>" > public/_default/index.html
```

---

## Testing Plan

### Unit Tests

1. `VHostResolver::normalize_authority()` - port stripping, lowercase
2. `VHostResolver::is_domain_directory()` - valid/invalid domain patterns
3. `VHostResolver::resolve()` - exact, wildcard, default, fallback

### Integration Tests

```bash
# Test exact match
curl -H "Host: simonedelpopolo.host" https://localhost/

# Test subdomain
curl -H "Host: api.simonedelpopolo.host" https://localhost/

# Test unknown domain (should use _default or legacy)
curl -H "Host: unknown.example.com" https://localhost/

# Test with port in authority
curl -H "Host: simonedelpopolo.host:443" https://localhost/
```

---

## Future Enhancements (Out of Scope)

1. **SNI-based TLS certificates** - Different certs per domain
2. **Per-vhost configuration** - Custom headers, redirects, auth
3. **Config file support** - JSON/YAML vhost definitions
4. **Admin API** - Runtime vhost management
5. **Per-vhost WebAuthn** - Different RP IDs per domain

---

## Files to Create/Modify

### New Files
| File | Purpose |
|------|---------|
| `include/neonsignal/vhost.h++` | VirtualHost types and VHostResolver class |
| `src/neonsignal/vhost/resolve.c++` | VHostResolver implementation |

### Modified Files
| File | Changes |
|------|---------|
| `include/neonsignal/router.h++` | Add resolve overload with document_root |
| `src/neonsignal/router/resolve.c++` | Implement resolve overload |
| `include/neonsignal/http2_listener.h++` | Add VHostResolver member |
| `src/neonsignal/http2_listener/handle_io_.c++` | Resolve vhost before static file load |
| `src/neonsignal/http2_listener/helper/load_static.c++` | Accept document_root parameter |
| `src/neonsignal/http2_listener/start.c++` | Initialize VHostResolver, log vhosts |
| `meson.build` | Add vhost source files |

---

## Implementation Order

1. **Create `vhost.h++`** - Types and class declaration
2. **Implement `resolve.c++`** - Core vhost resolution logic
3. **Update `meson.build`** - Add new source files
4. **Modify Router** - Add document_root overload
5. **Modify load_static** - Accept document_root parameter
6. **Integrate in handle_io_** - Wire up vhost resolution
7. **Add startup logging** - Show discovered vhosts
8. **Test** - Verify with curl and browser

**Estimated effort:** 2-3 hours for core implementation + testing

---

## Implementation Complete ✅

**Completed:** 2025-12-17

Name-based virtual hosting is now fully integrated. Here's how it works:

### How It Works

1. **Auto-discovery** - On startup, the server scans `public/` for directories that match domain patterns (contain at least one `.` like `domain.tld`). No config file needed!

2. **Request routing flow:**
   ```
   HTTP/2 Request → Extract :authority header → VHostResolver.resolve()
                         ↓
   "neonsignal.nutsloop.host" → public/neonsignal.nutsloop.host/
   "simonedelpopolo.host"     → public/simonedelpopolo.host/
   "unknown.com"              → public/_default/ (fallback)
   ```

3. **Port stripping** - `simonedelpopolo.host:443` normalizes to `simonedelpopolo.host`

4. **Case-insensitive** - `SiMoNeDelPoPolo.Host` matches `simonedelpopolo.host`

---

### Summary of Changes

#### New Files

| File | Purpose |
|------|---------|
| `include/neonsignal/vhost.h++` | VHostResolver class declaration |
| `src/neonsignal/vhost.c++` | Constructor |
| `src/neonsignal/vhost/resolve.c++` | Resolution logic |

#### Modified Files

| File | Changes |
|------|---------|
| `src/meson.build` | Added vhost sources |
| `include/neonsignal/router.h++` | Added document_root overload |
| `src/neonsignal/router/resolve.c++` | Implemented document_root overload |
| `include/neonsignal/http2_listener_helpers.h++` | Added `load_static_vhost()` declaration |
| `src/neonsignal/http2_listener/helper/load_static.c++` | Added `load_static_vhost()` implementation |
| `include/neonsignal/http2_listener.h++` | Added VHostResolver member |
| `src/neonsignal/http2_listener.c++` | Initialize VHostResolver in constructor |
| `src/neonsignal/http2_listener/handle_io_.c++` | Integrated vhost resolution for static files |
| `src/neonsignal/http2_listener/start.c++` | Added vhost discovery logging |
| `package.json` | Renamed build script to `neonsignal.nutsloop.host` |

---

### Final Directory Structure

```
public/
├── _default/                    # Fallback for unknown domains
│   └── index.html
├── neonsignal.nutsloop.host/    # Main app (SSE, auth, dashboard)
│   ├── app.js
│   ├── app.js.map
│   ├── css/
│   ├── favicon.svg
│   ├── icons/
│   ├── index.html
│   └── upload/
└── simonedelpopolo.host/        # Simple landing page
    └── index.html
```

---

### Using the New Build Script

```bash
# Build frontend to neonsignal.nutsloop.host vhost
npm run neonsignal.nutsloop.host
```

---

### Startup Log Output

```
neonsignal->Virtual hosts discovered:
  _default -> public/_default
  neonsignal.nutsloop.host -> public/neonsignal.nutsloop.host
  simonedelpopolo.host -> public/simonedelpopolo.host
```
