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
  [[nodiscard]] const std::optional<std::string> &www_root() const;
  [[nodiscard]] const std::optional<std::string> &certs_root() const;
  [[nodiscard]] const std::optional<std::string> &working_dir() const;

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
  std::optional<std::string> www_root_;
  std::optional<std::string> certs_root_;
  std::optional<std::string> working_dir_;
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

// ───────────────────────────────────────────────────────────────────────────
// INSTALL_VOLTAGE - Repository Installation Command Configuration
// ───────────────────────────────────────────────────────────────────────────

class install_voltage {
public:
  explicit install_voltage(int argc, char *argv[]);

  // Query methods
  [[nodiscard]] bool should_show_help() const;
  [[nodiscard]] bool should_show_version() const;
  [[nodiscard]] const std::optional<std::string> &help_text() const;
  [[nodiscard]] const std::optional<std::string> &version_text() const;

  // Configuration accessors
  [[nodiscard]] const std::optional<std::string> &repo() const;
  [[nodiscard]] const std::optional<std::string> &www_root() const;
  [[nodiscard]] const std::optional<std::string> &name() const;
  [[nodiscard]] const std::optional<std::string> &branch() const;
  [[nodiscard]] const std::optional<std::string> &only_save_path() const;

  // Systemd service
  [[nodiscard]] bool should_install_systemd_service() const;
  [[nodiscard]] const std::optional<std::string> &systemd_service() const;

private:
  static void validate_dash_format_(int argc, char *argv[]);

  bool show_help_{false};
  bool show_version_{false};
  bool should_install_systemd_service_{false};
  std::optional<std::string> help_text_;
  std::optional<std::string> version_text_;
  std::optional<std::string> repo_;
  std::optional<std::string> www_root_;
  std::optional<std::string> name_;
  std::optional<std::string> branch_;
  std::optional<std::string> only_save_path_;
  std::optional<std::string> systemd_service_;
};

} // namespace neonsignal
