#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <charconv>
#include <cctype>
#include <format>
#include <iostream>
#include <string_view>

namespace neonsignal::install {

namespace {

std::string trim_copy(std::string_view value) {
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
    value.remove_prefix(1);
  }
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
    value.remove_suffix(1);
  }
  return std::string(value);
}

bool parse_bool(std::string_view value, std::string_view label) {
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
    value.remove_prefix(1);
  }
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
    value.remove_suffix(1);
  }
  std::string lower;
  lower.reserve(value.size());
  for (unsigned char c : value) {
    lower.push_back(static_cast<char>(std::tolower(c)));
  }
  if (lower == "1" || lower == "true" || lower == "yes" || lower == "on") {
    return true;
  }
  if (lower == "0" || lower == "false" || lower == "no" || lower == "off") {
    return false;
  }
  throw std::invalid_argument(std::format("{} expects a boolean value", label));
}

} // namespace

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

    std::string key = trim_copy(pair.substr(0, colon_pos));
    std::string value = trim_copy(pair.substr(colon_pos + 1));

    if (key.empty()) {
      std::cerr << ansi("✗").bright_red().bold().str() << " empty key in pair: "
                << ansi(std::string(pair)).bright_red().curly_underline().str() << "\n";
      return false;
    }

    // Map key to config field
    if (key == "user") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.user = value;
    } else if (key == "group") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.group = value;
    } else if (key == "working-dir") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.working_dir = value;
    } else if (key == "threads") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " invalid number for threads: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.threads = num;
    } else if (key == "host") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.host = value;
      // Also update redirect_host if not explicitly set later
      config_.redirect_host = value;
    } else if (key == "port") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " invalid number for port: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.port = num;
    } else if (key == "webauthn-domain") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.webauthn_domain = value;
    } else if (key == "webauthn-origin") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.webauthn_origin = value;
    } else if (key == "exec-path") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.exec_path = value;
    } else if (key == "mail-enabled") {
      try {
        config_.mail_enabled = parse_bool(value, "mail-enabled");
      } catch (const std::exception &ex) {
        std::cerr << ansi("✗").bright_red().bold().str()
                  << " " << ex.what() << "\n";
        return false;
      }
    } else if (key == "mail-domains") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.mail_domains = value;
    } else if (key == "mail-cookie-name") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.mail_cookie_name = value;
    } else if (key == "mail-cookie-ttl") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      unsigned long long num = 0;
      auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num);
      if (ec != std::errc{} || ptr != value.data() + value.size()) {
        std::cerr << ansi("✗").bright_red().bold().str()
                  << " invalid number for mail-cookie-ttl: "
                  << ansi(value).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.mail_cookie_ttl = num;
    } else if (key == "mail-url-hits") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.mail_url_hits = value;
    } else if (key == "mail-from") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.mail_from = value;
    } else if (key == "mail-to-extra") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.mail_to_extra = value;
    } else if (key == "mail-command") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.mail_command = value;
    } else if (key == "mail-allowed-ip") {
      config_.mail_allowed_ip = value;
    } else if (key == "mail-save-db") {
      try {
        config_.mail_save_db = parse_bool(value, "mail-save-db");
      } catch (const std::exception &ex) {
        std::cerr << ansi("✗").bright_red().bold().str()
                  << " " << ex.what() << "\n";
        return false;
      }
    } else if (key == "redirect-instances") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
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
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
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
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
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
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.redirect_host = value;
    } else if (key == "redirect-exec-path") {
      if (value.empty()) {
        std::cerr << ansi("✗").bright_red().bold().str() << " empty value for key: "
                  << ansi(key).bright_red().curly_underline().str() << "\n";
        return false;
      }
      config_.redirect_exec_path = value;
    } else {
      std::cerr << ansi("✗").bright_red().bold().str() << " unknown key: "
                << ansi(key).bright_red().curly_underline().str() << "\n";
      std::cerr << ansi("▸").bright_cyan().str()
                << " Valid keys: user, group, working-dir, threads, host, port, webauthn-domain, "
                   "webauthn-origin, exec-path, mail-enabled, mail-domains, mail-cookie-name, "
                   "mail-cookie-ttl, mail-url-hits, mail-from, mail-to-extra, mail-command, "
                   "mail-allowed-ip, mail-save-db, redirect-instances, redirect-port, "
                   "redirect-target-port, redirect-host, redirect-exec-path\n";
      return false;
    }

    pos = pipe_pos + 1;
  }

  const bool mail_setting_present =
      config_.mail_domains.has_value() ||
      config_.mail_cookie_name.has_value() ||
      config_.mail_cookie_ttl.has_value() ||
      config_.mail_url_hits.has_value() ||
      config_.mail_from.has_value() ||
      config_.mail_to_extra.has_value() ||
      config_.mail_command.has_value() ||
      config_.mail_allowed_ip.has_value() ||
      config_.mail_save_db.has_value();

  if (mail_setting_present && !config_.mail_enabled.has_value()) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " mail keys were provided but mail-enabled is missing\n";
    return false;
  }

  return true;
}

} // namespace neonsignal::install
