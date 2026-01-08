#include "neonsignal/http2_listener.h++"
#include "neonsignal/event_loop.h++"
#include "neonsignal/event_mask.h++"

#include "neonsignal/http2_listener_helpers.h++"

#include <iostream>

namespace neonsignal {

void Http2Listener::start() {
  setup_listener_();

  // Preload static files into memory cache
  std::cerr << "Preloading static file cache from " << config_.public_root << "...\n";
  static_cache_->preload(config_.public_root);

  // Log virtual hosts
  if (vhost_resolver_.enabled()) {
    std::cerr << "neonsignal->Virtual hosts discovered:\n";
    for (const auto& vhost : vhost_resolver_.list_vhosts()) {
      std::cerr << "  " << vhost << '\n';
    }
  } else {
    std::cerr << "neonsignal->No virtual hosts configured (single-root mode)\n";
  }

  loop_.add_fd(listen_fd_, EventMask::Read, [this](std::uint32_t events) {
    if (events & (EventMask::Error | EventMask::HangUp)) {
      std::cerr << "Listener socket error/hup\n";
      return;
    }
    if (events & EventMask::Read) {
      handle_accept_();
    }
  });

  // Setup periodic timeout checking (every 5 seconds)
  loop_.add_timer(std::chrono::milliseconds(5000), [this]() {
    auto timed_out = conn_manager_->find_timed_out_connections();
    for (int fd : timed_out) {
      std::cerr << "Connection timeout, closing fd=" << fd << '\n';
      close_connection_(fd);
    }
  });

  start_redirect_monitor_();
}

} // namespace neonsignal
