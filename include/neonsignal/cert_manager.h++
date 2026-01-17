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
  std::filesystem::path cert_path;
  std::filesystem::path key_path;
  std::unique_ptr<SSL_CTX, void (*)(SSL_CTX *)> ssl_ctx{nullptr, SSL_CTX_free};
  bool is_wildcard{false};

  std::string common_name;
  std::vector<std::string> san_names;
  std::time_t not_before{0};
  std::time_t not_after{0};
};

class CertManager {
public:
  explicit CertManager(std::filesystem::path certs_root);

  bool initialize();
  SSL_CTX *get_context(std::string_view hostname) const;
  SSL_CTX *get_default_context() const;
  bool reload();
  [[nodiscard]] std::vector<std::string> list_certificates() const;
  [[nodiscard]] std::vector<std::string> expiring_soon(int days = 30) const;

private:
  std::filesystem::path certs_root_;
  mutable std::shared_mutex mutex_;

  std::unordered_map<std::string, std::unique_ptr<CertificateBundle>>
      exact_certs_;
  std::vector<std::unique_ptr<CertificateBundle>> wildcard_certs_;
  CertificateBundle *default_cert_{nullptr};

  std::unique_ptr<CertificateBundle>
  load_certificate(const std::filesystem::path &cert_dir,
                   const std::string &domain);

  bool configure_ssl_ctx(CertificateBundle &bundle);
  bool extract_cert_info(CertificateBundle &bundle);

  static std::string normalize_hostname(std::string_view hostname);
  static bool is_cert_directory(const std::filesystem::path &path);
};

} // namespace neonsignal
