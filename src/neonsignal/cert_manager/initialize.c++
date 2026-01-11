#include "neonsignal/cert_manager.h++"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <mutex>

#include <openssl/err.h>
#include <openssl/x509v3.h>

namespace {

// Select HTTP/2 via ALPN callback
int select_h2_alpn(SSL *, const unsigned char **out, unsigned char *outlen, const unsigned char *in,
                   unsigned int inlen, void *) {
  static const unsigned char alpn_h2[] = {0x02, 'h', '2'};

  unsigned char *selected = nullptr;
  unsigned char selected_len = 0;
  int rc = SSL_select_next_proto(&selected, &selected_len, alpn_h2, sizeof(alpn_h2), in, inlen);
  if (rc == OPENSSL_NPN_NEGOTIATED) {
    *out = selected;
    *outlen = selected_len;
    return SSL_TLSEXT_ERR_OK;
  }
  return SSL_TLSEXT_ERR_NOACK;
}

} // namespace

namespace neonsignal {

bool CertManager::initialize() {
  std::unique_lock lock(mutex_);
  exact_certs_.clear();
  wildcard_certs_.clear();
  default_cert_ = nullptr;

  if (!std::filesystem::is_directory(certs_root_)) {
    std::cerr << "neonsignal->CertManager: certs directory not found: " << certs_root_ << '\n';
    return false;
  }

  std::cerr << "neonsignal->CertManager scanning: " << certs_root_ << '\n';

  for (const auto &entry : std::filesystem::directory_iterator(certs_root_)) {
    if (!entry.is_directory()) {
      continue;
    }
    if (!is_cert_directory(entry.path())) {
      continue;
    }

    std::string name = entry.path().filename().string();
    auto bundle = load_certificate(entry.path(), name);

    if (!bundle) {
      std::cerr << "neonsignal->CertManager: failed to load cert for " << name << '\n';
      continue;
    }

    if (name == "_default") {
      default_cert_ = bundle.get();
      exact_certs_["_default"] = std::move(bundle);
    } else if (name.starts_with("*.")) {
      bundle->is_wildcard = true;
      bundle->domain = name.substr(2);
      wildcard_certs_.push_back(std::move(bundle));
    } else {
      std::string normalized = normalize_hostname(name);
      exact_certs_[normalized] = std::move(bundle);
    }
  }

  if (!default_cert_) {
    std::cerr << "neonsignal->CertManager: WARNING - no _default certificate found\n";
    // Use first available cert as fallback
    if (!exact_certs_.empty()) {
      for (auto &[name, bundle] : exact_certs_) {
        if (name != "_default") {
          default_cert_ = bundle.get();
          std::cerr << "neonsignal->CertManager: using " << name << " as fallback default\n";
          break;
        }
      }
    }
  }

  return default_cert_ != nullptr;
}

bool CertManager::is_cert_directory(const std::filesystem::path &path) {
  return std::filesystem::exists(path / "fullchain.pem") &&
         std::filesystem::exists(path / "privkey.pem");
}

std::unique_ptr<CertificateBundle>
CertManager::load_certificate(const std::filesystem::path &cert_dir, const std::string &domain) {
  auto bundle = std::make_unique<CertificateBundle>();
  bundle->domain = domain;
  bundle->cert_path = cert_dir / "fullchain.pem";
  bundle->key_path = cert_dir / "privkey.pem";

  if (!configure_ssl_ctx(*bundle)) {
    return nullptr;
  }

  if (!extract_cert_info(*bundle)) {
    std::cerr << "neonsignal->CertManager: warning - could not extract cert "
                 "info for "
              << domain << '\n';
  }

  return bundle;
}

bool CertManager::configure_ssl_ctx(CertificateBundle &bundle) {
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    return false;
  }

  bundle.ssl_ctx.reset(ctx);

  // Modern TLS settings
  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

  // Prefer ECDHE cipher suites
  if (SSL_CTX_set_cipher_list(ctx, "ECDHE:!aNULL") != 1) {
    std::cerr << "neonsignal->CertManager: failed to configure cipher list for " << bundle.domain
              << '\n';
    return false;
  }

  // Load certificate chain
  if (SSL_CTX_use_certificate_chain_file(ctx, bundle.cert_path.c_str()) != 1) {
    std::cerr << "neonsignal->CertManager: failed to load certificate: " << bundle.cert_path
              << '\n';
    ERR_print_errors_fp(stderr);
    return false;
  }

  // Load private key
  if (SSL_CTX_use_PrivateKey_file(ctx, bundle.key_path.c_str(), SSL_FILETYPE_PEM) != 1) {
    std::cerr << "neonsignal->CertManager: failed to load private key: " << bundle.key_path << '\n';
    ERR_print_errors_fp(stderr);
    return false;
  }

  // Verify key matches certificate
  if (SSL_CTX_check_private_key(ctx) != 1) {
    std::cerr << "neonsignal->CertManager: private key does not match "
                 "certificate for "
              << bundle.domain << '\n';
    return false;
  }

  // Configure ALPN for HTTP/2
  SSL_CTX_set_alpn_select_cb(ctx, select_h2_alpn, nullptr);

  std::cerr << "neonsignal->CertManager: loaded certificate for " << bundle.domain << '\n';
  return true;
}

bool CertManager::extract_cert_info(CertificateBundle &bundle) {
  if (!bundle.ssl_ctx) {
    return false;
  }

  SSL *ssl = SSL_new(bundle.ssl_ctx.get());
  if (!ssl) {
    return false;
  }

  X509 *cert = SSL_get_certificate(ssl);
  if (!cert) {
    SSL_free(ssl);
    return false;
  }

  // Extract CN
  X509_NAME *subject = X509_get_subject_name(cert);
  if (subject) {
    char cn_buf[256] = {0};
    X509_NAME_get_text_by_NID(subject, NID_commonName, cn_buf, sizeof(cn_buf));
    bundle.common_name = cn_buf;
  }

  // Extract validity dates
  const ASN1_TIME *not_before = X509_get0_notBefore(cert);
  const ASN1_TIME *not_after = X509_get0_notAfter(cert);

  if (not_before) {
    struct tm tm_before = {};
    ASN1_TIME_to_tm(not_before, &tm_before);
    bundle.not_before = std::mktime(&tm_before);
  }
  if (not_after) {
    struct tm tm_after = {};
    ASN1_TIME_to_tm(not_after, &tm_after);
    bundle.not_after = std::mktime(&tm_after);
  }

  // Extract SANs
  GENERAL_NAMES *san_names =
      static_cast<GENERAL_NAMES *>(X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));
  if (san_names) {
    int san_count = sk_GENERAL_NAME_num(san_names);
    for (int i = 0; i < san_count; ++i) {
      GENERAL_NAME *entry = sk_GENERAL_NAME_value(san_names, i);
      if (entry->type == GEN_DNS) {
        const char *dns_name =
            reinterpret_cast<const char *>(ASN1_STRING_get0_data(entry->d.dNSName));
        if (dns_name) {
          bundle.san_names.emplace_back(dns_name);
        }
      } else if (entry->type == GEN_IPADD) {
        // Handle IP SANs
        ASN1_OCTET_STRING *ip = entry->d.iPAddress;
        if (ip && ip->length == 4) {
          char ip_str[16];
          snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip->data[0], ip->data[1], ip->data[2],
                   ip->data[3]);
          bundle.san_names.emplace_back(ip_str);
        }
      }
    }
    GENERAL_NAMES_free(san_names);
  }

  SSL_free(ssl);
  return true;
}

bool CertManager::reload() {
  // Clear and reinitialize
  return initialize();
}

std::vector<std::string> CertManager::list_certificates() const {
  std::shared_lock lock(mutex_);
  std::vector<std::string> result;

  for (const auto &[name, bundle] : exact_certs_) {
    std::string entry = name + " -> " + bundle->cert_path.string();
    if (!bundle->common_name.empty()) {
      entry += " (CN=" + bundle->common_name + ")";
    }
    if (bundle.get() == default_cert_) {
      entry += " [default]";
    }
    result.push_back(std::move(entry));
  }

  for (const auto &bundle : wildcard_certs_) {
    std::string entry = "*." + bundle->domain + " -> " + bundle->cert_path.string();
    if (!bundle->common_name.empty()) {
      entry += " (CN=" + bundle->common_name + ")";
    }
    result.push_back(std::move(entry));
  }

  return result;
}

std::vector<std::string> CertManager::expiring_soon(int days) const {
  std::shared_lock lock(mutex_);
  std::vector<std::string> result;

  auto now = std::time(nullptr);
  auto threshold = now + (days * 24 * 60 * 60);

  auto check_bundle = [&](const CertificateBundle &bundle) {
    if (bundle.not_after > 0 && bundle.not_after < threshold) {
      int days_left = static_cast<int>((bundle.not_after - now) / (24 * 60 * 60));
      result.push_back(bundle.domain + " expires in " + std::to_string(days_left) + " days");
    }
  };

  for (const auto &[name, bundle] : exact_certs_) {
    check_bundle(*bundle);
  }
  for (const auto &bundle : wildcard_certs_) {
    check_bundle(*bundle);
  }

  return result;
}

} // namespace neonsignal
