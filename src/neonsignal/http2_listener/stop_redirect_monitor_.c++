#include "neonsignal/http2_listener.h++"
#include "neonsignal/event_loop.h++"

namespace neonsignal {

void Http2Listener::stop_redirect_monitor_() {
  if (redirect_timer_id_ != -1) {
    loop_.cancel_timer(redirect_timer_id_);
    redirect_timer_id_ = -1;
  }
}

} // namespace neonsignal
