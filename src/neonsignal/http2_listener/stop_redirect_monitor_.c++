#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener.h++"

namespace neonsignal {

void Http2Listener::stop_redirect_monitor_() {
  if (redirect_timer_fd_ != -1) {
    loop_.remove_fd(redirect_timer_fd_);
    close(redirect_timer_fd_);
    redirect_timer_fd_ = -1;
  }
}

} // namespace neonsignal
