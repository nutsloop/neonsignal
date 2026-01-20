#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <charconv>
#include <iostream>
#include <string_view>

namespace neonsignal::install {

bool SystemdServiceInstaller::parse_kvp_(const std::string &kvp_string) {
  using nutsloop::ansi;

  std::size_t pos = 0;

  while (pos < kvp_string.length()) {
    // Find next pipe delimiter or end of string
    std::size_t pipe_pos = kvp_string.find('|', pos);
    if (pipe_pos == std::string::npos) {
      pipe_pos = kvp_string.length();
    }

    std::string_view pair = std::string_view(kvp_string).substr(pos, pipe_pos - pos);

    // Find colon separator (first occurrence, value may contain colons)
    std::size_t colon_pos = pair.find(':');
    if (colon_pos == std::string_view::npos) {
      std::cerr << ansi("✗").bright_red().bold().str()
                << " malformed key:value pair: "
                << ansi(std::string(pair)).bright_red().curly_underline().str()
                << " (missing colon)\n";
      return false;
    }

    std::string key(pair.substr(0, colon_pos));
    std::string value(pair.substr(colon_pos + 1));

    if (key.empty()) {
      std::cerr << ansi("✗").bright_red().bold().str() << " empty key in pair: "
                << ansi(std::string(pair)).bright_red().curly_underline().str() << "\n";
      return false;
    }

    if (value.empty()) {
      std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                << ansi(key).bright_red().curly_underline().str() << "\n";
      return false;
    }

    // Map key to config field
    if (key == "user") {
      config_.user = value;
    } else if (key == "group") {
      config_.group = value;
    } else if (key == "working-dir") {
      config_.working_dir = value;
    } else if (key == "threads") {
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " invalid number for threads: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.threads = num;
    } else if (key == "host") {
      config_.host = value;
      // Also update redirect_host if not explicitly set later
      config_.redirect_host = value;
    } else if (key == "port") {
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " invalid number for port: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.port = num;
    } else if (key == "webauthn-domain") {
      config_.webauthn_domain = value;
    } else if (key == "webauthn-origin") {
      config_.webauthn_origin = value;
    } else if (key == "exec-path") {
      config_.exec_path = value;
    } else if (key == "redirect-instances") {
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str()
                  << " invalid number for redirect-instances: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.redirect_instances = num;
    } else if (key == "redirect-port") {
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str()
                  << " invalid number for redirect-port: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.redirect_port = num;
    } else if (key == "redirect-target-port") {
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str()
                  << " invalid number for redirect-target-port: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.redirect_target_port = num;
    } else if (key == "redirect-host") {
      config_.redirect_host = value;
    } else if (key == "redirect-exec-path") {
      config_.redirect_exec_path = value;
    } else {
      std::cerr << ansi("✗").bright_red().bold().str() << " unknown key: "
                << ansi(key).bright_red().curly_underline().str() << "\n";
      std::cerr << ansi("▸").bright_cyan().str()
                << " Valid keys: user, group, working-dir, threads, host, port, webauthn-domain, "
                   "webauthn-origin, exec-path, redirect-instances, redirect-port, "
                   "redirect-target-port, redirect-host, redirect-exec-path\n";
      return false;
    }

    pos = pipe_pos + 1;
  }

  return true;
}

} // namespace neonsignal::install
