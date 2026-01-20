#include "spin/event_loop.h++"

#include <utility>

namespace neonsignal {

void EventLoop::add_fd(int fd, std::uint32_t events,
                       std::function<void(std::uint32_t)> callback) {
  backend_->add_fd(fd, events, std::move(callback));

  std::lock_guard<std::mutex> lock(callbacks_mutex_);
  callbacks_[fd] = nullptr;  // Just track that we have this fd
}

} // namespace neonsignal
