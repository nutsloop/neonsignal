#include "neonsignal/event_loop_backend.h++"
#include "neonsignal/event_mask.h++"

#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <format>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace neonsignal {

namespace {

// Convert OS-neutral EventMask to epoll flags
std::uint32_t to_epoll_events(std::uint32_t mask) {
  std::uint32_t events = 0;
  if (mask & EventMask::Read) events |= EPOLLIN;
  if (mask & EventMask::Write) events |= EPOLLOUT;
  if (mask & EventMask::Error) events |= EPOLLERR;
  if (mask & EventMask::HangUp) events |= EPOLLHUP;
  if (mask & EventMask::Edge) events |= EPOLLET;
  if (mask & EventMask::ReadHangUp) events |= EPOLLRDHUP;
  return events;
}

// Convert epoll flags to OS-neutral EventMask
std::uint32_t from_epoll_events(std::uint32_t events) {
  std::uint32_t mask = 0;
  if (events & EPOLLIN) mask |= EventMask::Read;
  if (events & EPOLLOUT) mask |= EventMask::Write;
  if (events & EPOLLERR) mask |= EventMask::Error;
  if (events & EPOLLHUP) mask |= EventMask::HangUp;
  if (events & EPOLLET) mask |= EventMask::Edge;
  if (events & EPOLLRDHUP) mask |= EventMask::ReadHangUp;
  return mask;
}

} // anonymous namespace

class LinuxEventLoopBackend : public EventLoopBackend {
public:
  LinuxEventLoopBackend() = default;
  ~LinuxEventLoopBackend() override { cleanup(); }

  void init() override {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ == -1) {
      throw std::runtime_error("failed to create epoll instance");
    }
  }

  void cleanup() override {
    // Close timer fds
    for (auto& [id, info] : timers_) {
      if (info.fd != -1) {
        close(info.fd);
      }
    }
    timers_.clear();

    // Close signal fd
    if (signal_fd_ != -1) {
      close(signal_fd_);
      signal_fd_ = -1;
    }

    // Close epoll fd
    if (epoll_fd_ != -1) {
      close(epoll_fd_);
      epoll_fd_ = -1;
    }
  }

  void add_fd(int fd, std::uint32_t events,
              std::function<void(std::uint32_t)> callback) override {
    epoll_event ev{};
    ev.events = to_epoll_events(events);
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
      throw std::runtime_error(
          std::format("epoll_ctl add failed: {}", std::strerror(errno)));
    }

    std::lock_guard lock(mutex_);
    callbacks_[fd] = std::move(callback);
  }

  void update_fd(int fd, std::uint32_t events) override {
    epoll_event ev{};
    ev.events = to_epoll_events(events);
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
      throw std::runtime_error(
          std::format("epoll_ctl mod failed: {}", std::strerror(errno)));
    }
  }

  void remove_fd(int fd) override {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    std::lock_guard lock(mutex_);
    callbacks_.erase(fd);
  }

  int add_timer(std::chrono::milliseconds interval,
                std::function<void()> callback) override {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd == -1) {
      throw std::runtime_error(
          std::format("timerfd_create failed: {}", std::strerror(errno)));
    }

    struct itimerspec spec;
    std::memset(&spec, 0, sizeof(spec));

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(interval);
    auto nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(interval - seconds);

    spec.it_interval.tv_sec = seconds.count();
    spec.it_interval.tv_nsec = nanoseconds.count();
    spec.it_value = spec.it_interval;

    if (timerfd_settime(timer_fd, 0, &spec, nullptr) == -1) {
      close(timer_fd);
      throw std::runtime_error(
          std::format("timerfd_settime failed: {}", std::strerror(errno)));
    }

    int timer_id = next_timer_id_++;
    {
      std::lock_guard lock(mutex_);
      timers_[timer_id] = {timer_fd, std::move(callback)};
    }

    // Register timer fd with epoll
    add_fd(timer_fd, EventMask::Read, [this, timer_id](std::uint32_t) {
      std::function<void()> cb;
      int fd = -1;
      {
        std::lock_guard lock(mutex_);
        auto it = timers_.find(timer_id);
        if (it != timers_.end()) {
          fd = it->second.fd;
          cb = it->second.callback;
        }
      }
      if (fd != -1) {
        // Read to clear the timerfd event
        std::uint64_t expirations;
        ::read(fd, &expirations, sizeof(expirations));
      }
      if (cb) {
        cb();
      }
    });

    return timer_id;
  }

  void cancel_timer(int timer_id) override {
    std::lock_guard lock(mutex_);
    auto it = timers_.find(timer_id);
    if (it != timers_.end()) {
      if (it->second.fd != -1) {
        remove_fd(it->second.fd);
        close(it->second.fd);
      }
      timers_.erase(it);
    }
  }

  int poll(int timeout_ms) override {
    std::array<epoll_event, 64> events{};
    int n = epoll_wait(epoll_fd_, events.data(),
                       static_cast<int>(events.size()), timeout_ms);
    if (n == -1) {
      if (errno == EINTR) {
        return 0;
      }
      return -1;
    }

    for (int i = 0; i < n; ++i) {
      int fd = events[i].data.fd;
      std::function<void(std::uint32_t)> cb;
      {
        std::lock_guard lock(mutex_);
        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) {
          cb = it->second;
        }
      }
      if (cb) {
        cb(from_epoll_events(events[i].events));
      }
    }

    return n;
  }

  void add_signal(int signum, std::function<void()> callback) override {
    if (!signal_mask_initialized_) {
      sigemptyset(&signal_mask_);
      signal_mask_initialized_ = true;
    }
    sigaddset(&signal_mask_, signum);

    if (sigprocmask(SIG_BLOCK, &signal_mask_, nullptr) == -1) {
      throw std::runtime_error("failed to block signal");
    }

    if (signal_fd_ == -1) {
      signal_fd_ = signalfd(-1, &signal_mask_, SFD_NONBLOCK | SFD_CLOEXEC);
    } else {
      // Add to existing signalfd
      signal_fd_ = signalfd(signal_fd_, &signal_mask_, SFD_NONBLOCK | SFD_CLOEXEC);
    }

    if (signal_fd_ == -1) {
      throw std::runtime_error("failed to create signalfd");
    }

    {
      std::lock_guard lock(mutex_);
      signal_callbacks_[signum] = std::move(callback);
    }

    // Only add to epoll once
    if (!signal_fd_registered_) {
      add_fd(signal_fd_, EventMask::Read, [this](std::uint32_t) {
        signalfd_siginfo info;
        while (::read(signal_fd_, &info, sizeof(info)) == sizeof(info)) {
          std::function<void()> cb;
          {
            std::lock_guard lock(mutex_);
            auto it = signal_callbacks_.find(static_cast<int>(info.ssi_signo));
            if (it != signal_callbacks_.end()) {
              cb = it->second;
            }
          }
          if (cb) {
            cb();
          }
        }
      });
      signal_fd_registered_ = true;
    }
  }

private:
  struct TimerInfo {
    int fd{-1};
    std::function<void()> callback;
  };

  int epoll_fd_{-1};
  int signal_fd_{-1};
  int next_timer_id_{1};
  bool signal_mask_initialized_{false};
  bool signal_fd_registered_{false};
  sigset_t signal_mask_{};
  std::mutex mutex_;
  std::unordered_map<int, std::function<void(std::uint32_t)>> callbacks_;
  std::unordered_map<int, TimerInfo> timers_;
  std::unordered_map<int, std::function<void()>> signal_callbacks_;
};

std::unique_ptr<EventLoopBackend> create_event_loop_backend() {
  auto backend = std::make_unique<LinuxEventLoopBackend>();
  backend->init();
  return backend;
}

} // namespace neonsignal
