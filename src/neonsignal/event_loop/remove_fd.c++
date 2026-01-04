#include "neonsignal/event_loop.h++"

#include <sys/epoll.h>

namespace neonsignal {

void EventLoop::remove_fd(int fd) {
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
  std::lock_guard<std::mutex> lock(callbacks_mutex_);
  callbacks_.erase(fd);
}

} // namespace neonsignal
