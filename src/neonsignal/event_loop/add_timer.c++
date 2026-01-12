#include "neonsignal/event_loop.h++"

#include <cstring>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace neonsignal {

void EventLoop::add_timer(std::chrono::milliseconds interval, std::function<void()> callback) {
  if (timer_fd_ != -1) {
    throw std::runtime_error("Timer already registered");
  }

  // Create timerfd
  timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer_fd_ == -1) {
    throw std::runtime_error(std::format("timerfd_create failed: {}", std::strerror(errno)));
  }

  // Set timer to fire periodically
  struct itimerspec spec;
  std::memset(&spec, 0, sizeof(spec));

  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(interval);
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(interval - seconds);

  spec.it_interval.tv_sec = seconds.count();
  spec.it_interval.tv_nsec = nanoseconds.count();
  spec.it_value = spec.it_interval; // Initial expiration

  if (timerfd_settime(timer_fd_, 0, &spec, nullptr) == -1) {
    close(timer_fd_);
    timer_fd_ = -1;
    throw std::runtime_error(std::format("timerfd_settime failed: {}", std::strerror(errno)));
  }

  // Register with epoll
  timer_callback_ = std::move(callback);

  add_fd(timer_fd_, EPOLLIN, [this](std::uint32_t) {
    // Read to clear the timerfd event
    std::uint64_t expirations;
    ::read(timer_fd_, &expirations, sizeof(expirations));

    // Invoke callback
    if (timer_callback_) {
      timer_callback_();
    }
  });
}

} // namespace neonsignal
