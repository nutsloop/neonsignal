#include "neonsignal/logging.h++"
#include "neonsignal/neonsignal.h++"
#include "neonsignal/voltage_argv.h++"

#include <ansi.h++>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

namespace {

bool env_set(const char *name) { return std::getenv(name) != nullptr; }

bool port_in_range(unsigned long long value) { return value > 0 && value <= 65535; }

} // namespace

int main(int argc, char *argv[]) {
  neonsignal::server_voltage *voltage_ptr = nullptr;

  try {
    voltage_ptr = new neonsignal::server_voltage(argc, argv);
    const neonsignal::server_voltage &voltage = *voltage_ptr;

    if (voltage.should_show_help()) {
      if (voltage.help_text()) {
        std::cout << *voltage.help_text() << '\n';
      }
      delete voltage_ptr;
      return 0;
    }
    if (voltage.should_show_version()) {
      if (voltage.version_text()) {
        std::cout << *voltage.version_text() << '\n';
      }
      delete voltage_ptr;
      return 0;
    }

    neonsignal::ServerConfig config;

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

    if (!env_set("NEONSIGNAL_THREADS") && voltage.threads() && *voltage.threads() > 0) {
      const auto threads_value = std::to_string(*voltage.threads());
      setenv("NEONSIGNAL_THREADS", threads_value.c_str(), 1);
    }

    delete voltage_ptr;

    neonsignal::install_thread_logging_prefix();
    neonsignal::Server server(config);
    server.run();
  } catch (const std::exception &ex) {
    using nutsloop::ansi;

    // Styled error message
    std::cerr << ansi("✗ Fatal Error").bright_red().bold().str() << "\n";
    std::cerr << "  "
              << ansi(ex.what()).bright_red().curly_underline().underline_color({255, 165, 0}).str()
              << "\n\n";

    // Show help hint
    std::cerr << ansi("ℹ For usage information, run:").bright_cyan().str() << "\n";
    std::cerr << "  " << ansi(argv[0]).bright_yellow().str() << " ";
    std::cerr << ansi("--help").bright_white().str() << "\n";

    // Clean up
    if (voltage_ptr != nullptr) {
      delete voltage_ptr;
    }

    return 1;
  }

  return 0;
}
