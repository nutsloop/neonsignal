#include "neonsignal/event_loop.h++"

#include <sys/epoll.h>
#include <unistd.h>

#include <stdexcept>

namespace neonsignal {

EventLoop::EventLoop() : epoll_fd_(epoll_create1(EPOLL_CLOEXEC)) {
  if (epoll_fd_ == -1) {
    throw std::runtime_error("failed to create epoll instance");
  }
}

EventLoop::~EventLoop() {
  stop();
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
  }
}

} // namespace neonsignal
