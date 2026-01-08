#include "neonsignal/http2_listener.h++"
#include "neonsignal/event_loop.h++"
#include "neonsignal/event_mask.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <chrono>
#include <iostream>
#include <thread>

namespace neonsignal {

void Http2Listener::start_redirect_monitor_() {
  if (redirect_timer_id_ != -1) {
    return;
  }

  redirect_timer_id_ = loop_.add_timer(std::chrono::seconds(1), [this]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    bool ok = probe_redirect_service_();
    redirect_service_ok_.store(ok);

    // Broadcast redirect status to all Redirect channel subscribers
    std::string body = "data: {\"redirect_ok\": " +
                       std::string(ok ? "true" : "false") + "}\n\n";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());

    sse_broadcaster_->for_each_subscriber(SSEBroadcaster::Channel::Redirect,
      [&](const std::shared_ptr<Http2Connection>& c, std::uint32_t stream_id) {
        // Skip connections with write backpressure
        if (conn_manager_->has_write_backpressure(c)) {
          return;
        }
        auto data_frame = build_frame(0x0 /* DATA */, 0x0, stream_id, body_bytes);
        c->write_buf.insert(c->write_buf.end(), data_frame.begin(), data_frame.end());
        c->events |= EventMask::Write;
        loop_.update_fd(c->fd, c->events);
      });
  });
}

} // namespace neonsignal
