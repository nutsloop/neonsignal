#include "spin/event_loop.h++"

#include <utility>

namespace neonsignal {

int EventLoop::add_timer(std::chrono::milliseconds interval,
                         std::function<void()> callback) {
  return backend_->add_timer(interval, std::move(callback));
}

} // namespace neonsignal
