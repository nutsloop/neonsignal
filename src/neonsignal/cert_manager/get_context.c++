#include "neonsignal/cert_manager.h++"

#include <algorithm>
#include <cctype>
#include <shared_mutex>

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

SSL_CTX *CertManager::get_context(std::string_view hostname) const {
  std::shared_lock lock(mutex_);

  std::string normalized = normalize_hostname(hostname);

  // 1. Exact match
  if (auto it = exact_certs_.find(normalized); it != exact_certs_.end()) {
    return it->second->ssl_ctx.get();
  }

  // 2. Wildcard match (*.example.com matches sub.example.com)
  for (const auto &wc : wildcard_certs_) {
    if (normalized.size() > wc->domain.size() + 1) {
      auto suffix_start = normalized.size() - wc->domain.size();
      if (normalized[suffix_start - 1] == '.' && normalized.substr(suffix_start) == wc->domain) {
        return wc->ssl_ctx.get();
      }
    }
  }

  // 3. Check if hostname is covered by any cert's SAN
  for (const auto &[name, bundle] : exact_certs_) {
    for (const auto &san : bundle->san_names) {
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

SSL_CTX *CertManager::get_default_context() const {
  std::shared_lock lock(mutex_);
  return default_cert_ ? default_cert_->ssl_ctx.get() : nullptr;
}

} // namespace neonsignal
