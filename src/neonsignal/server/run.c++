#include "neonsignal/neonsignal.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener.h++"
#include "neonsignal/router.h++"
#include "neonsignal/thread_pool.h++"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

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
  router_ = std::make_unique<Router>(config_.public_root);
  listener_ = std::make_unique<Http2Listener>(*loop_, *pool_, ssl_ctx_.get(), config_, *router_,
                                              served_files_, page_views_, event_clients_);
  listener_->start();

  // Graceful shutdown on SIGINT/SIGTERM using signalfd.
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);
  if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
    throw std::runtime_error("failed to block signals");
  }
  shutdown_fd_ = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
  if (shutdown_fd_ == -1) {
    throw std::runtime_error("failed to create signalfd");
  }
  loop_->add_fd(shutdown_fd_, EPOLLIN, [this](std::uint32_t) { stop(); });
  loop_->run();

  if (shutdown_fd_ != -1) {
    loop_->remove_fd(shutdown_fd_);
    close(shutdown_fd_);
    shutdown_fd_ = -1;
  }
}

} // namespace neonsignal
