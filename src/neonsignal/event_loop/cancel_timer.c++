#include "neonsignal/event_loop.h++"

namespace neonsignal {

void EventLoop::cancel_timer(int timer_id) {
  backend_->cancel_timer(timer_id);
}

} // namespace neonsignal
