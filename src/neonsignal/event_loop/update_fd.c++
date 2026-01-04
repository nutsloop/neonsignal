#include "neonsignal/event_loop.h++"

#include <sys/epoll.h>

#include <stdexcept>

namespace neonsignal {

void EventLoop::update_fd(int fd, std::uint32_t events) {
  epoll_event ev{};
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
    throw std::runtime_error("epoll_ctl mod failed");
  }
}

} // namespace neonsignal
