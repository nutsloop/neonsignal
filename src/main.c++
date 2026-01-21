#include "install/install_command.h++"
#include "install/systemd_service_installer.h++"
#include "neonsignal/logging.h++"
#include "neonsignal/voltage_argv.h++"
#include "spin/neonsignal.h++"

#include <ansi.h++>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

#include <unistd.h>

namespace {

bool env_set(const char *name) { return std::getenv(name) != nullptr; }

bool port_in_range(unsigned long long value) { return value > 0 && value <= 65535; }

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
