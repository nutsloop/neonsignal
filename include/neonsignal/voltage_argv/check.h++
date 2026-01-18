#pragma once

#include "args/types/args_key_value_t.h++"

#include <string>

namespace neonsignal::voltage_argv {

using nutsloop::args::args_key_value_t_;

class check {
public:
  explicit check(args_key_value_t_ value = nullptr);

  ~check() = default;

  // Server options
  [[nodiscard]] unsigned long long threads() const;
  [[nodiscard]] std::string host() const;
  [[nodiscard]] unsigned long long port() const;
  [[nodiscard]] std::string webauthn_domain() const;
  [[nodiscard]] std::string webauthn_origin() const;
  [[nodiscard]] std::string db_path() const;
  [[nodiscard]] std::string www_root() const;
  [[nodiscard]] std::string certs_root() const;
  [[nodiscard]] std::string working_dir() const;

  // shared
  [[nodiscard]] bool systemd() const;

  // Redirect options
  [[nodiscard]] unsigned long long instances() const;
  [[nodiscard]] unsigned long long redirect_port() const;
  [[nodiscard]] unsigned long long target_port() const;
  [[nodiscard]] std::string redirect_host() const;
  [[nodiscard]] std::string acme_webroot() const;

private:
  args_key_value_t_ value_;
};

} // namespace neonsignal::voltage_argv
