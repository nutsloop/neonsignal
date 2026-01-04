#include "neonsignal/event_loop.h++"

#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <format>

namespace neonsignal {

void EventLoop::shutdown_graceful(std::chrono::seconds timeout) {
  std::cerr << std::format("⏹️  Graceful shutdown initiated (timeout: {}s)\n",
                           timeout.count());

  shutdown_requested_.store(true, std::memory_order_release);

  auto deadline = std::chrono::steady_clock::now() + timeout;

  // Continue processing events until all FDs are closed or timeout
  while (active_fd_count() > 0 &&
         std::chrono::steady_clock::now() < deadline) {

    // Process events with 1 second timeout
    struct epoll_event events[16];
    int nfds = epoll_wait(epoll_fd_, events, 16, 1000);

    if (nfds > 0) {
      std::lock_guard lock(callbacks_mutex_);
      for (int i = 0; i < nfds; ++i) {
        int fd = events[i].data.fd;
        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) {
          it->second(events[i].events);
        }
      }
    }

    std::cerr << std::format("⏳ Draining connections... {} remaining\n",
                             active_fd_count());
  }

  if (active_fd_count() > 0) {
    std::cerr << std::format("⚠️  Timeout reached, {} connections force-closed\n",
                             active_fd_count());
  } else {
    std::cerr << "✅ All connections drained successfully\n";
  }

  stop();
}

} // namespace neonsignal
