#pragma once

#include "args/types/version_t.h++"

#include <string>
#include <unordered_map>

namespace neonsignal::voltage_argv {

class help {

  enum class Topic_ {
    // Main help
    help,
    version,

    // Server options
    threads,
    host,
    port,
    webauthn_domain,
    webauthn_origin,
    db_path,
    systemd,

    // Redirect options
    instances,
    target_port,
    acme_webroot,

    // Command
    spin,

    // Unknown
    unknown
  };

public:
  explicit help(std::string look_up_option, nutsloop::args::version_t version);

  [[nodiscard]] int print();
  [[nodiscard]] std::string to_string();

private:
  std::string look_up_option_;
  nutsloop::args::version_t version_info_;

  using topic_enum_t_ = std::unordered_map<std::string, Topic_>;
  topic_enum_t_ option_list_;

  void set_option_list_();
  void remove_leading_hyphens_();
  [[nodiscard]] Topic_ stoenum_();
  [[nodiscard]] std::string get_version_() const;
  [[nodiscard]] std::string get_help_topic_(Topic_ topic) const;

  // Help topic methods
  [[nodiscard]] std::string help_() const;
  [[nodiscard]] std::string version_() const;
  [[nodiscard]] std::string threads_() const;
  [[nodiscard]] std::string host_() const;
  [[nodiscard]] std::string port_() const;
  [[nodiscard]] std::string webauthn_domain_() const;
  [[nodiscard]] std::string webauthn_origin_() const;
  [[nodiscard]] std::string db_path_() const;
  [[nodiscard]] std::string systemd_() const;
  [[nodiscard]] std::string instances_() const;
  [[nodiscard]] std::string target_port_() const;
  [[nodiscard]] std::string acme_webroot_() const;
  [[nodiscard]] std::string spin_() const;
  [[nodiscard]] std::string unknown_() const;
};

} // namespace neonsignal::voltage_argv
