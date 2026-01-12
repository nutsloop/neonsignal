#pragma once

#include <optional>
#include <string>

namespace neonsignal {

// ───────────────────────────────────────────────────────────────────────────
// SERVER_VOLTAGE - Main HTTP/2 Server Configuration
// ───────────────────────────────────────────────────────────────────────────

class server_voltage {
public:
  explicit server_voltage(int argc, char *argv[]);

  // Query methods
  [[nodiscard]] bool is_systemd() const;
  [[nodiscard]] bool should_show_help() const;
  [[nodiscard]] bool should_show_version() const;
  [[nodiscard]] const std::optional<std::string> &help_text() const;
  [[nodiscard]] const std::optional<std::string> &version_text() const;

  // Configuration accessors
  [[nodiscard]] const std::optional<unsigned long long> &threads() const;
  [[nodiscard]] const std::optional<std::string> &host() const;
  [[nodiscard]] const std::optional<unsigned long long> &port() const;
  [[nodiscard]] const std::optional<std::string> &webauthn_domain() const;
  [[nodiscard]] const std::optional<std::string> &webauthn_origin() const;
  [[nodiscard]] const std::optional<std::string> &db_path() const;

private:
  static void validate_dash_format_(int argc, char *argv[]);

  bool systemd_{false};
  bool show_help_{false};
  bool show_version_{false};
  std::optional<std::string> help_text_;
  std::optional<std::string> version_text_;
  std::optional<unsigned long long> threads_;
  std::optional<std::string> host_;
  std::optional<unsigned long long> port_;
  std::optional<std::string> webauthn_domain_;
  std::optional<std::string> webauthn_origin_;
  std::optional<std::string> db_path_;
};

// ───────────────────────────────────────────────────────────────────────────
// REDIRECT_VOLTAGE - HTTP→HTTPS Redirector Configuration
// ───────────────────────────────────────────────────────────────────────────

class redirect_voltage {
public:
  explicit redirect_voltage(int argc, char *argv[]);

  // Query methods
  [[nodiscard]] bool is_systemd() const;
  [[nodiscard]] bool should_show_help() const;
  [[nodiscard]] bool should_show_version() const;
  [[nodiscard]] const std::optional<std::string> &help_text() const;
  [[nodiscard]] const std::optional<std::string> &version_text() const;

  // Configuration accessors
  [[nodiscard]] const std::optional<unsigned long long> &instances() const;
  [[nodiscard]] const std::optional<unsigned long long> &port() const;
  [[nodiscard]] const std::optional<unsigned long long> &target_port() const;
  [[nodiscard]] const std::optional<std::string> &host() const;
  [[nodiscard]] const std::optional<std::string> &acme_webroot() const;

private:
  static void validate_dash_format_(int argc, char *argv[]);

  bool systemd_{false};
  bool show_help_{false};
  bool show_version_{false};
  std::optional<std::string> help_text_;
  std::optional<std::string> version_text_;
  std::optional<unsigned long long> instances_;
  std::optional<unsigned long long> port_;
  std::optional<unsigned long long> target_port_;
  std::optional<std::string> host_;
  std::optional<std::string> acme_webroot_;
};

} // namespace neonsignal
