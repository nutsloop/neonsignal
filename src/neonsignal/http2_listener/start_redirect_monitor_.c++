#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <chrono>
#include <iostream>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace neonsignal {

void Http2Listener::start_redirect_monitor_() {
  if (redirect_timer_fd_ != -1) {
    return;
  }

  redirect_timer_fd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
  if (redirect_timer_fd_ == -1) {
    std::cerr << "failed to create redirect monitor timer\n";
    return;
  }

  itimerspec spec{};
  spec.it_interval.tv_sec = 1;
  spec.it_value.tv_sec = 1;
  if (timerfd_settime(redirect_timer_fd_, 0, &spec, nullptr) == -1) {
    std::cerr << "failed to start redirect monitor timer\n";
    close(redirect_timer_fd_);
    redirect_timer_fd_ = -1;
    return;
  }

  loop_.add_fd(redirect_timer_fd_, EPOLLIN, [this](std::uint32_t) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::uint64_t expirations = 0;
    if (read(redirect_timer_fd_, &expirations, sizeof(expirations)) < 0) {
      return;
    }
    bool ok = probe_redirect_service_();
    redirect_service_ok_.store(ok);

    // Broadcast redirect status to all Redirect channel subscribers
    std::string body = "data: {\"redirect_ok\": " + std::string(ok ? "true" : "false") + "}\n\n";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());

    sse_broadcaster_->for_each_subscriber(
        SSEBroadcaster::Channel::Redirect,
        [&](const std::shared_ptr<Http2Connection> &c, std::uint32_t stream_id) {
          // Skip connections with write backpressure
          if (conn_manager_->has_write_backpressure(c)) {
            return;
          }
          auto data_frame = build_frame(0x0 /* DATA */, 0x0, stream_id, body_bytes);
          c->write_buf.insert(c->write_buf.end(), data_frame.begin(), data_frame.end());
          c->events |= EPOLLOUT;
          loop_.update_fd(c->fd, c->events);
        });
  });
}

} // namespace neonsignal
