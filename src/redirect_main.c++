#include "neonsignal/logging.h++"
#include "neonsignal/redirect_service.h++"

#include <exception>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <string>
#include <thread>
#include <vector>

namespace {

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

int main() {
  try {
    neonsignal::install_thread_logging_prefix();
    const int instances = read_int_env("REDIRECT_INSTANCES", 1);
    const int listen_port = read_int_env("REDIRECT_PORT", 9090);
    const int target_port = read_int_env("REDIRECT_TARGET_PORT", 443);
    const char *host_env = std::getenv("REDIRECT_HOST");
    std::string host = host_env ? host_env : "0.0.0.0";
    const char *acme_env = std::getenv("ACME_WEBROOT");
    std::string acme_root = acme_env ? acme_env : "acme-challenge";

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
        services.push_back(
            std::make_unique<neonsignal::RedirectService>(listen_port, host, target_port, acme_root));
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
    std::cerr << "Redirect fatal: " << ex.what() << '\n';
    return 1;
  }

  return 0;
}
