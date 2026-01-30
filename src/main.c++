#include "install/install_command.h++"
#include "install/systemd_service_installer.h++"
#include "neonsignal/logging.h++"
#include "neonsignal/voltage_argv.h++"
#include "spin/neonsignal.h++"

#include <ansi.h++>

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <unistd.h>

namespace {

bool env_set(const char *name) { return std::getenv(name) != nullptr; }

bool port_in_range(unsigned long long value) { return value > 0 && value <= 65535; }

std::string trim_copy(std::string_view value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) {
    return {};
  }
  const auto last = value.find_last_not_of(" \t\r\n");
  return std::string(value.substr(first, last - first + 1));
}

std::vector<std::string> split_list(std::string_view value) {
  std::vector<std::string> out;
  std::size_t start = 0;
  while (start <= value.size()) {
    std::size_t comma = value.find(',', start);
    if (comma == std::string_view::npos) {
      comma = value.size();
    }
    auto item = trim_copy(value.substr(start, comma - start));
    if (!item.empty()) {
      out.push_back(std::move(item));
    }
    if (comma == value.size()) {
      break;
    }
    start = comma + 1;
  }
  return out;
}

bool parse_bool(std::string_view value, std::string_view label) {
  auto trimmed = trim_copy(value);
  if (trimmed.empty()) {
    throw std::runtime_error(std::format("{} expects a boolean value", label));
  }
  std::string lower;
  lower.reserve(trimmed.size());
  for (unsigned char c : trimmed) {
    lower.push_back(static_cast<char>(std::tolower(c)));
  }
  if (lower == "1" || lower == "true" || lower == "yes" || lower == "on") {
    return true;
  }
  if (lower == "0" || lower == "false" || lower == "no" || lower == "off") {
    return false;
  }
  throw std::runtime_error(std::format("{} expects a boolean value", label));
}

// Check if the first non-flag argument is "install"
bool is_install_command(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    // Skip flags (arguments starting with -)
    if (arg.starts_with("-")) {
      continue;
    }
    // First non-flag argument is the command
    return arg == "install";
  }
  return false;
}

int run_install_command(int argc, char *argv[]) {
  if (argc == 2 && std::string_view(argv[1]) == "install") {
    using nutsloop::ansi;

    std::cerr << ansi("✗").bright_red().bold().str()
              << " install requires one of these switch sets:" << '\n';
    std::cerr << ansi("  ▸").bright_cyan().str() << " "
              << ansi(std::format("{} install", argv[0])).bright_yellow().str() << '\n';
    std::cerr << ansi("    ↳").bright_cyan().str() << " "
              << ansi("--systemd-service").bright_green().str() << '\n';
    std::cerr << ansi("    ↳").bright_cyan().str() << " "
              << ansi("--only-save[=<path>]").bright_green().str() << '\n';
    std::cerr << ansi("  ▸").bright_cyan().str() << " "
              << ansi(std::format("{} install", argv[0])).bright_yellow().str() << '\n';
    std::cerr << ansi("    ↳").bright_cyan().str() << " " << ansi("--repo").bright_green().str()
              << '\n';
    std::cerr << ansi("    ↳").bright_cyan().str() << " " << ansi("--www-root").bright_green().str()
              << '\n';
    std::cerr << ansi("    ↳").bright_cyan().str() << " " << ansi("--name").bright_green().str()
              << '\n';
    std::cerr << ansi("    ↳").bright_cyan().str() << " " << ansi("--branch").bright_green().str()
              << '\n';

    return 1;
  }

  neonsignal::install_voltage voltage(argc, argv);

  if (voltage.should_show_help()) {
    if (voltage.help_text()) {
      std::cout << *voltage.help_text() << '\n';
    }
    return 0;
  }
  if (voltage.should_show_version()) {
    if (voltage.version_text()) {
      std::cout << *voltage.version_text() << '\n';
    }
    return 0;
  }

  if (voltage.should_install_systemd_service()) {
#if !defined(__linux__)
    using nutsloop::ansi;

    if (!voltage.only_save_path()) {
      std::cerr << ansi("✗").bright_red().bold().str()
                << " systemd-service installs are only supported on Linux\n";
      std::cerr << ansi("▸").bright_cyan().str()
                << " use --only-save to generate service files on this platform\n";
      return 1;
    }
#endif
    neonsignal::install::SystemdServiceInstaller installer(voltage.systemd_service(),
                                                           voltage.only_save_path());
    return installer.install();
  }

  neonsignal::install::InstallCommand cmd(voltage);
  return cmd.run();
}

int run_spin_command(int argc, char *argv[]) {
  neonsignal::server_voltage voltage(argc, argv);

  if (voltage.should_show_help()) {
    if (voltage.help_text()) {
      std::cout << *voltage.help_text() << '\n';
    }
    return 0;
  }
  if (voltage.should_show_version()) {
    if (voltage.version_text()) {
      std::cout << *voltage.version_text() << '\n';
    }
    return 0;
  }

  // Change working directory if specified (before resolving other paths)
  // Env var takes precedence over CLI arg
  std::string working_dir_path;
  if (const char *env = std::getenv("NEONSIGNAL_WORKING_DIR"); env != nullptr) {
    working_dir_path = env;
  } else if (voltage.working_dir() && !voltage.working_dir()->empty()) {
    working_dir_path = *voltage.working_dir();
  }

  if (!working_dir_path.empty()) {
    std::filesystem::path path(working_dir_path);
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
      throw std::runtime_error(
          std::format("working directory does not exist: '{}'", working_dir_path));
    }
    if (access(working_dir_path.c_str(), W_OK) != 0) {
      throw std::runtime_error(
          std::format("working directory is not writable: '{}'", working_dir_path));
    }
    try {
      std::filesystem::current_path(working_dir_path);
    } catch (const std::filesystem::filesystem_error &e) {
      throw std::runtime_error(std::format("failed to change working directory to '{}': {}",
                                           working_dir_path, e.what()));
    }
  }

  neonsignal::ServerConfig config;

  // Store resolved working directory in config
  config.working_dir = std::filesystem::current_path().string();

  if (!env_set("NEONSIGNAL_HOST") && voltage.host() && !voltage.host()->empty()) {
    config.host = *voltage.host();
  }
  if (!env_set("NEONSIGNAL_PORT") && voltage.port() && port_in_range(*voltage.port())) {
    config.port = static_cast<std::uint16_t>(*voltage.port());
  }
  if (!env_set("NEONSIGNAL_WEBAUTHN_DOMAIN") && voltage.webauthn_domain() &&
      !voltage.webauthn_domain()->empty()) {
    config.rp_id = *voltage.webauthn_domain();
  }
  if (!env_set("NEONSIGNAL_WEBAUTHN_ORIGIN") && voltage.webauthn_origin() &&
      !voltage.webauthn_origin()->empty()) {
    config.origin = *voltage.webauthn_origin();
  }
  if (!env_set("NEONSIGNAL_DB_PATH") && voltage.db_path() && !voltage.db_path()->empty()) {
    config.db_path = *voltage.db_path();
  }
  if (!env_set("NEONSIGNAL_WWW_ROOT") && voltage.www_root() && !voltage.www_root()->empty()) {
    config.www_root = *voltage.www_root();
  }
  if (!env_set("NEONSIGNAL_CERTS_ROOT") && voltage.certs_root() && !voltage.certs_root()->empty()) {
    config.certs_root = *voltage.certs_root();
  }

  const bool mail_enabled_explicit = (std::getenv("NEONSIGNAL_MAIL_ENABLED") != nullptr) ||
                                     voltage.mail_enabled().has_value();
  const bool mail_settings_present =
      (std::getenv("NEONSIGNAL_MAIL_DOMAINS") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_COOKIE_NAME") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_COOKIE_TTL") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_URL_HITS") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_FROM") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_TO_EXTRA") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_COMMAND") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_ALLOWED_IP") != nullptr) ||
      (std::getenv("NEONSIGNAL_MAIL_SAVE_DB") != nullptr) ||
      voltage.mail_domains().has_value() ||
      voltage.mail_cookie_name().has_value() ||
      voltage.mail_cookie_ttl().has_value() ||
      voltage.mail_url_hits().has_value() ||
      voltage.mail_from().has_value() ||
      voltage.mail_to_extra().has_value() ||
      voltage.mail_command().has_value() ||
      voltage.mail_allowed_ip().has_value() ||
      voltage.mail_save_db().has_value();

  if (mail_settings_present && !mail_enabled_explicit) {
    throw std::runtime_error(
        "mail settings provided but NEONSIGNAL_MAIL_ENABLED/--mail-enabled is missing");
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_ENABLED"); env != nullptr) {
    config.mail.enabled = parse_bool(env, "NEONSIGNAL_MAIL_ENABLED");
  } else if (voltage.mail_enabled()) {
    config.mail.enabled = *voltage.mail_enabled();
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_DOMAINS"); env != nullptr) {
    config.mail.allowed_domains = split_list(env);
  } else if (voltage.mail_domains()) {
    config.mail.allowed_domains = split_list(*voltage.mail_domains());
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_COOKIE_NAME"); env != nullptr) {
    auto name = trim_copy(env);
    if (name.empty()) {
      throw std::runtime_error("NEONSIGNAL_MAIL_COOKIE_NAME cannot be empty");
    }
    config.mail.cookie_name = std::move(name);
  } else if (voltage.mail_cookie_name()) {
    auto name = trim_copy(*voltage.mail_cookie_name());
    if (name.empty()) {
      throw std::runtime_error("--mail-cookie-name cannot be empty");
    }
    config.mail.cookie_name = std::move(name);
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_COOKIE_TTL"); env != nullptr) {
    try {
      config.mail.cookie_lifespan = std::chrono::seconds(std::stoull(env));
    } catch (const std::exception &) {
      throw std::runtime_error("NEONSIGNAL_MAIL_COOKIE_TTL expects integer seconds");
    }
  } else if (voltage.mail_cookie_ttl()) {
    config.mail.cookie_lifespan = std::chrono::seconds(*voltage.mail_cookie_ttl());
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_URL_HITS"); env != nullptr) {
    auto hits = split_list(env);
    for (auto &hit : hits) {
      if (!hit.empty() && hit.front() != '/') {
        hit.insert(hit.begin(), '/');
      }
    }
    config.mail.url_hits = std::move(hits);
  } else if (voltage.mail_url_hits()) {
    auto hits = split_list(*voltage.mail_url_hits());
    for (auto &hit : hits) {
      if (!hit.empty() && hit.front() != '/') {
        hit.insert(hit.begin(), '/');
      }
    }
    config.mail.url_hits = std::move(hits);
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_FROM"); env != nullptr) {
    auto from = split_list(env);
    if (from.empty()) {
      throw std::runtime_error("NEONSIGNAL_MAIL_FROM must include at least one address");
    }
    config.mail.from_addresses = std::move(from);
  } else if (voltage.mail_from()) {
    auto from = split_list(*voltage.mail_from());
    if (from.empty()) {
      throw std::runtime_error("--mail-from must include at least one address");
    }
    config.mail.from_addresses = std::move(from);
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_TO_EXTRA"); env != nullptr) {
    config.mail.to_extra = split_list(env);
  } else if (voltage.mail_to_extra()) {
    config.mail.to_extra = split_list(*voltage.mail_to_extra());
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_COMMAND"); env != nullptr) {
    auto command = trim_copy(env);
    if (command.empty()) {
      throw std::runtime_error("NEONSIGNAL_MAIL_COMMAND cannot be empty");
    }
    config.mail.mail_command = std::move(command);
  } else if (voltage.mail_command()) {
    auto command = trim_copy(*voltage.mail_command());
    if (command.empty()) {
      throw std::runtime_error("--mail-command cannot be empty");
    }
    config.mail.mail_command = std::move(command);
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_ALLOWED_IP"); env != nullptr) {
    config.mail.allowed_ip_address = trim_copy(env);
  } else if (voltage.mail_allowed_ip()) {
    config.mail.allowed_ip_address = trim_copy(*voltage.mail_allowed_ip());
  }

  if (const char *env = std::getenv("NEONSIGNAL_MAIL_SAVE_DB"); env != nullptr) {
    config.mail.save_to_database = parse_bool(env, "NEONSIGNAL_MAIL_SAVE_DB");
  } else if (voltage.mail_save_db()) {
    config.mail.save_to_database = *voltage.mail_save_db();
  }

  if (!env_set("NEONSIGNAL_THREADS") && voltage.threads() && *voltage.threads() > 0) {
    const auto threads_value = std::to_string(*voltage.threads());
    setenv("NEONSIGNAL_THREADS", threads_value.c_str(), 1);
  }

  neonsignal::install_thread_logging_prefix();
  neonsignal::Server server(config);
  server.run();

  return 0;
}

} // namespace

int main(int argc, char *argv[]) {
  try {
    // Check if this is an install command
    if (is_install_command(argc, argv)) {
      return run_install_command(argc, argv);
    }

    // Default to spin command
    return run_spin_command(argc, argv);
  } catch (const std::exception &ex) {
    using nutsloop::ansi;

    // Styled error message
    std::cerr << ansi("✗").bright_red().bold().str() << " Fatal\n";
    std::cerr << ansi("↳").bright_red().str() << " "
              << ansi(ex.what()).bright_red().curly_underline().underline_color({255, 165, 0}).str()
              << "\n\n";

    // Show help hint
    std::cerr << ansi("▸").bright_cyan().str() << " For usage information, run:\n";
    std::cerr << ansi("↳").bright_yellow().str() << " " << ansi(argv[0]).bright_yellow().str()
              << " " << ansi("--help").bright_white().str() << "\n";

    return 1;
  }
}
