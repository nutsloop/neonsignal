#include "spin/neonsignal.h++"

#include "spin/event_loop.h++"
#include "spin/http2_listener.h++"
#include "spin/router.h++"
#include "spin/thread_pool.h++"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace neonsignal {

void Server::run() {
  if (!ssl_ctx_) {
    initialize_tls();
  }

  configure_credentials();

  if (const char *rp = std::getenv("NEONSIGNAL_WEBAUTHN_DOMAIN")) {
    config_.rp_id = rp;
  }
  if (const char *origin = std::getenv("NEONSIGNAL_WEBAUTHN_ORIGIN")) {
    config_.origin = origin;
  }
  if (const char *db_path = std::getenv("NEONSIGNAL_DB_PATH")) {
    config_.db_path = db_path;
  }
  if (const char *www_root = std::getenv("NEONSIGNAL_WWW_ROOT")) {
    config_.www_root = www_root;
  }
  if (const char *certs_root = std::getenv("NEONSIGNAL_CERTS_ROOT")) {
    config_.certs_root = certs_root;
  }
  if (const char *host = std::getenv("NEONSIGNAL_HOST")) {
    config_.host = host;
  }
  if (const char *port_env = std::getenv("NEONSIGNAL_PORT")) {
    try {
      unsigned long v = std::stoul(port_env);
      if (v > 0 && v <= 65535) {
        config_.port = static_cast<std::uint16_t>(v);
      }
    } catch (...) {
    }
  }

  auto thread_count = std::thread::hardware_concurrency();
  if (const char *env_threads = std::getenv("NEONSIGNAL_THREADS")) {
    try {
      unsigned long v = std::stoul(env_threads);
      if (v > 0) {
        thread_count = static_cast<unsigned int>(v);
      }
    } catch (...) {
    }
  }
  if (thread_count == 0) {
    thread_count = 2;
  }
  ThreadPool::ServerHostPort server_host_port = {.host = config_.host, .port = config_.port};
  pool_ = std::make_unique<ThreadPool>(thread_count, server_host_port);
  loop_ = std::make_unique<EventLoop>();
  router_ = std::make_unique<Router>(config_.www_root);
  listener_ = std::make_unique<Http2Listener>(*loop_, *pool_, ssl_ctx_.get(), config_, *router_,
                                              served_files_, page_views_, event_clients_);
  listener_->start();

  // Graceful shutdown on SIGINT/SIGTERM using portable signal handling
  loop_->add_signal(SIGINT, [this]() { stop(); });
  loop_->add_signal(SIGTERM, [this]() { stop(); });

  loop_->run();
}

} // namespace neonsignal
