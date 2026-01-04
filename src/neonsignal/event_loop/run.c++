#include "neonsignal/event_loop.h++"

#include <sys/epoll.h>

#include <array>
#include <cerrno>
#include <stdexcept>

namespace neonsignal {

void EventLoop::run() {
  running_.store(true);
  std::array<epoll_event, 64> events{};

  while (running_.load()) {
    int n = epoll_wait(epoll_fd_, events.data(),
                       static_cast<int>(events.size()), 500);
    if (n == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("epoll_wait failed");
    }

    for (int i = 0; i < n; ++i) {
      int fd = events[i].data.fd;
      std::function<void(std::uint32_t)> cb;
      {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) {
          cb = it->second;
        }
      }
      if (cb) {
        cb(events[i].events);
      }
    }
  }
}

} // namespace neonsignal
