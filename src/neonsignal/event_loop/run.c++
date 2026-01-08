#include "neonsignal/event_loop.h++"

#include <stdexcept>

namespace neonsignal {

void EventLoop::run() {
  running_.store(true);

  while (running_.load()) {
    int n = backend_->poll(500);  // 500ms timeout
    if (n == -1) {
      throw std::runtime_error("event loop poll failed");
    }
  }
}

} // namespace neonsignal
