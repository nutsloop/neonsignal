#include "neonsignal/logging.h++"
#include "neonsignal/redirect_service.h++"
#include "neonsignal/voltage_argv.h++"

#include <ansi.h++>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <string>
#include <thread>
#include <vector>

namespace {

bool env_set(const char *name) { return std::getenv(name) != nullptr; }

int read_int_env(const char *name, const int def) {
  if (const char *val = std::getenv(name)) {
    try {
      if (const int v = std::stoi(val); v > 0) {
        return v;
      }
    } catch (...) {
    }
  }
  return def;
}

void set_thread_name(const std::string &n) {
  std::string name = n;
  if (name.size() > 15) {
    name.resize(15);
  }
  pthread_setname_np(pthread_self(), name.c_str());
}

} // namespace

int main(int argc, char *argv[]) {
  neonsignal::redirect_voltage *voltage_ptr = nullptr;

  try {
    voltage_ptr = new neonsignal::redirect_voltage(argc, argv);
    const neonsignal::redirect_voltage &voltage = *voltage_ptr;

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

    neonsignal::install_thread_logging_prefix();

    int instances = read_int_env("REDIRECT_INSTANCES", 1);
    int listen_port = read_int_env("REDIRECT_PORT", 9090);
    int target_port = read_int_env("REDIRECT_TARGET_PORT", 443);
    std::string host = env_set("REDIRECT_HOST") ? std::getenv("REDIRECT_HOST") : "0.0.0.0";
    std::string acme_root =
        env_set("ACME_WEBROOT") ? std::getenv("ACME_WEBROOT") : "acme-challenge";

    if (!env_set("REDIRECT_INSTANCES") && voltage.instances() && *voltage.instances() > 0) {
      instances = static_cast<int>(*voltage.instances());
    }
    if (!env_set("REDIRECT_PORT") && voltage.port() && *voltage.port() > 0 &&
        *voltage.port() <= 65535) {
      listen_port = static_cast<int>(*voltage.port());
    }
    if (!env_set("REDIRECT_TARGET_PORT") && voltage.target_port() && *voltage.target_port() > 0 &&
        *voltage.target_port() <= 65535) {
      target_port = static_cast<int>(*voltage.target_port());
    }
    if (!env_set("REDIRECT_HOST") && voltage.host() && !voltage.host()->empty()) {
      host = *voltage.host();
    }
    if (!env_set("ACME_WEBROOT") && voltage.acme_webroot() && !voltage.acme_webroot()->empty()) {
      acme_root = *voltage.acme_webroot();
    }

    delete voltage_ptr;

    if (instances <= 1) {
      set_thread_name("redir-main");
      neonsignal::RedirectService service(listen_port, host, target_port, acme_root);
      service.start();
    } else {
      std::vector<std::unique_ptr<neonsignal::RedirectService>> services;
      std::vector<std::thread> threads;
      services.reserve(static_cast<std::size_t>(instances));
      threads.reserve(static_cast<std::size_t>(instances));
      for (int i = 0; i < instances; ++i) {
        services.push_back(std::make_unique<neonsignal::RedirectService>(listen_port, host,
                                                                         target_port, acme_root));
      }
      for (std::size_t idx = 0; idx < services.size(); ++idx) {
        auto &svc = services[idx];
        threads.emplace_back([&svc, idx]() {
          std::string thread_name = std::format("neonredrec->({})", std::to_string(idx));
          set_thread_name(thread_name);
          svc->start();
        });
      }
      for (auto &t : threads) {
        if (t.joinable()) {
          t.join();
        }
      }
    }
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
