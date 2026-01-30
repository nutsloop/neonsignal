#include "spin/http2_listener.h++"
#include "spin/event_loop.h++"

#include <iostream>
#include <unistd.h>

namespace neonsignal {

void Http2Listener::shutdown_graceful() {
  // Stop the redirect monitor timer
  stop_redirect_monitor_();

  // Stop the connection timeout timer
  if (timeout_timer_id_ != -1) {
    loop_.cancel_timer(timeout_timer_id_);
    timeout_timer_id_ = -1;
  }
  if (mail_cookie_timer_id_ != -1) {
    loop_.cancel_timer(mail_cookie_timer_id_);
    mail_cookie_timer_id_ = -1;
  }

  // Stop accepting new connections
  if (listen_fd_ != -1) {
    loop_.remove_fd(listen_fd_);
    close(listen_fd_);
    listen_fd_ = -1;
    std::cerr << "• Listener socket closed, no longer accepting connections\n";
  }
}

} // namespace neonsignal
