#include "neonsignal/event_loop.h++"

#include <utility>

namespace neonsignal {

void EventLoop::add_signal(int signum, std::function<void()> callback) {
  backend_->add_signal(signum, std::move(callback));
}

} // namespace neonsignal
