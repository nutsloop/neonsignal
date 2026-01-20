#include "spin/event_loop.h++"

namespace neonsignal {

void EventLoop::remove_fd(int fd) {
  backend_->remove_fd(fd);
  std::lock_guard<std::mutex> lock(callbacks_mutex_);
  callbacks_.erase(fd);
}

} // namespace neonsignal
