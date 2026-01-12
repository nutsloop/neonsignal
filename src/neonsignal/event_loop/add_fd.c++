#include "neonsignal/event_loop.h++"

#include <sys/epoll.h>

#include <stdexcept>
#include <utility>

namespace neonsignal {

void EventLoop::add_fd(int fd, std::uint32_t events, std::function<void(std::uint32_t)> callback) {
  epoll_event ev{};
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    throw std::runtime_error("epoll_ctl add failed");
  }

  std::lock_guard<std::mutex> lock(callbacks_mutex_);
  callbacks_[fd] = std::move(callback);
}

} // namespace neonsignal
