#pragma once

#include <optional>
#include <string>

namespace neonsignal::install {

struct SystemdServiceConfig {
  // neonsignal.service properties
  std::string user;
  std::string group;
  std::string working_dir;
  unsigned long long threads{3};
  std::string host{"0.0.0.0"};
  unsigned long long port{9443};
  std::optional<std::string> webauthn_domain;
  std::optional<std::string> webauthn_origin;
  std::string exec_path{"/usr/local/bin/neonsignal"};

  // neonsignal_redirect.service properties
  unsigned long long redirect_instances{3};
  unsigned long long redirect_port{9090};
  unsigned long long redirect_target_port{443};
  std::string redirect_host;
  std::string redirect_exec_path{"/usr/local/bin/neonsignal_redirect"};
};

class SystemdServiceInstaller {
public:
  SystemdServiceInstaller(const std::optional<std::string> &kvp_string,
                          const std::optional<std::string> &only_save_path);

  [[nodiscard]] int install();

private:
  SystemdServiceConfig config_;
  bool using_defaults_{true};
  std::optional<std::string> only_save_path_;

  void populate_defaults_();
  [[nodiscard]] bool parse_kvp_(const std::string &kvp_string);
  [[nodiscard]] bool validate_config_() const;
  [[nodiscard]] bool check_root_privileges_() const;
  [[nodiscard]] std::string generate_neonsignal_service_() const;
  [[nodiscard]] std::string generate_redirect_service_() const;
  [[nodiscard]] bool write_service_file_(const std::string &path, const std::string &content) const;
  void print_defaults_warning_() const;
  void print_success_message_() const;
};

} // namespace neonsignal::install
