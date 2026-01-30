#include "spin/http2_listener.h++"
#include "spin/event_loop.h++"
#include "spin/event_mask.h++"

#include "spin/http2_listener_helpers.h++"
#include "spin/mail_cookie_store.h++"

#include <filesystem>
#include <iostream>

namespace neonsignal {

void Http2Listener::start() {
  setup_listener_();

  // Log working directory
  std::cerr << "• neonsignal->Working directory: " << std::filesystem::current_path().string() << '\n';

  // Preload static files into memory cache
  std::cerr << "• Preloading static file cache from " << config_.www_root << "...\n";
  static_cache_->preload(config_.www_root);

  // Log virtual hosts
  if (vhost_resolver_.enabled()) {
    std::cerr << "• neonsignal->Virtual hosts discovered:\n";
    for (const auto& vhost : vhost_resolver_.list_vhosts()) {
      std::cerr << "↳ " << vhost << '\n';
    }
  } else {
    std::cerr << "• neonsignal->No virtual hosts configured (single-root mode)\n";
  }

  std::cerr << "• mail: /api/mail " << (config_.mail.enabled ? "enabled" : "disabled");
  if (config_.mail.enabled) {
    std::cerr << " domains=";
    if (config_.mail.allowed_domains.empty()) {
      std::cerr << "none";
    } else {
      for (std::size_t i = 0; i < config_.mail.allowed_domains.size(); ++i) {
        if (i > 0) {
          std::cerr << ",";
        }
        std::cerr << config_.mail.allowed_domains[i];
      }
    }
    std::cerr << " cmd=" << config_.mail.mail_command
              << " cookie=" << config_.mail.cookie_name
              << " ttl=" << config_.mail.cookie_lifespan.count() << "s"
              << " url_hits=";
    if (config_.mail.url_hits.empty()) {
      std::cerr << "none";
    } else {
      for (std::size_t i = 0; i < config_.mail.url_hits.size(); ++i) {
        if (i > 0) {
          std::cerr << ",";
        }
        std::cerr << config_.mail.url_hits[i];
      }
    }
    if (!config_.mail.allowed_ip_address.empty()) {
      std::cerr << " allow_ip=" << config_.mail.allowed_ip_address;
    }
    std::cerr << " save_db=" << (config_.mail.save_to_database ? "true" : "false");
  }
  std::cerr << '\n';

  loop_.add_fd(listen_fd_, EventMask::Read, [this](std::uint32_t events) {
    if (events & (EventMask::Error | EventMask::HangUp)) {
      std::cerr << "▲ Listener socket fault/hup\n";
      return;
    }
    if (events & EventMask::Read) {
      handle_accept_();
    }
  });

  // Setup periodic timeout checking (every 5 seconds)
  timeout_timer_id_ = loop_.add_timer(std::chrono::milliseconds(5000), [this]() {
    auto timed_out = conn_manager_->find_timed_out_connections();
    for (int fd : timed_out) {
      std::cerr << "▲ Connection timeout, closing fd=" << fd << '\n';
      close_connection_(fd);
    }
  });

  mail_cookie_timer_id_ = loop_.add_timer(std::chrono::seconds(60), [this]() {
    if (mail_cookie_store_) {
      mail_cookie_store_->cleanup_expired();
    }
  });

  start_redirect_monitor_();
}

} // namespace neonsignal
