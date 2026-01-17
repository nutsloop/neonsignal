#include "neonsignal/event_loop_backend.h++"
#include "neonsignal/event_mask.h++"

#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <format>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace neonsignal {

class DarwinEventLoopBackend : public EventLoopBackend {
public:
  DarwinEventLoopBackend() = default;
  ~DarwinEventLoopBackend() override { cleanup(); }

  void init() override {
    kqueue_fd_ = kqueue();
    if (kqueue_fd_ == -1) {
      throw std::runtime_error("failed to create kqueue");
    }
  }

  void cleanup() override {
    if (kqueue_fd_ != -1) {
      close(kqueue_fd_);
      kqueue_fd_ = -1;
    }
  }

  void add_fd(int fd, std::uint32_t events,
              std::function<void(std::uint32_t)> callback) override {
    std::vector<struct kevent> changes;

    // Determine flags based on edge triggering
    int flags = EV_ADD | EV_ENABLE;
    if (events & EventMask::Edge) {
      flags |= EV_CLEAR;  // EV_CLEAR is kqueue's edge-trigger equivalent
    }

    if (events & EventMask::Read) {
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_READ, flags, 0, 0, nullptr);
      changes.push_back(ev);
    }

    if (events & EventMask::Write) {
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_WRITE, flags, 0, 0, nullptr);
      changes.push_back(ev);
    }

    if (!changes.empty()) {
      if (kevent(kqueue_fd_, changes.data(), static_cast<int>(changes.size()),
                 nullptr, 0, nullptr) == -1) {
        throw std::runtime_error(
            std::format("kevent add failed: {}", std::strerror(errno)));
      }
    }

    std::lock_guard lock(mutex_);
    callbacks_[fd] = std::move(callback);
    fd_events_[fd] = events;
  }

  void update_fd(int fd, std::uint32_t events) override {
    std::uint32_t old_events = 0;
    {
      std::lock_guard lock(mutex_);
      auto it = fd_events_.find(fd);
      if (it != fd_events_.end()) {
        old_events = it->second;
      }
      fd_events_[fd] = events;
    }

    std::vector<struct kevent> changes;

    int flags = EV_ADD | EV_ENABLE;
    if (events & EventMask::Edge) {
      flags |= EV_CLEAR;
    }

    // Handle read filter
    if ((events & EventMask::Read) && !(old_events & EventMask::Read)) {
      // Adding read
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_READ, flags, 0, 0, nullptr);
      changes.push_back(ev);
    } else if (!(events & EventMask::Read) && (old_events & EventMask::Read)) {
      // Removing read
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
      changes.push_back(ev);
    }

    // Handle write filter
    if ((events & EventMask::Write) && !(old_events & EventMask::Write)) {
      // Adding write
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_WRITE, flags, 0, 0, nullptr);
      changes.push_back(ev);
    } else if (!(events & EventMask::Write) && (old_events & EventMask::Write)) {
      // Removing write
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
      changes.push_back(ev);
    }

    if (!changes.empty()) {
      if (kevent(kqueue_fd_, changes.data(), static_cast<int>(changes.size()),
                 nullptr, 0, nullptr) == -1) {
        throw std::runtime_error(
            std::format("kevent update failed: {}", std::strerror(errno)));
      }
    }
  }

  void remove_fd(int fd) override {
    std::uint32_t events = 0;
    {
      std::lock_guard lock(mutex_);
      auto it = fd_events_.find(fd);
      if (it != fd_events_.end()) {
        events = it->second;
        fd_events_.erase(it);
      }
      callbacks_.erase(fd);
    }

    std::vector<struct kevent> changes;

    if (events & EventMask::Read) {
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
      changes.push_back(ev);
    }

    if (events & EventMask::Write) {
      struct kevent ev;
      EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
      changes.push_back(ev);
    }

    if (!changes.empty()) {
      // Ignore errors on delete (fd might already be closed)
      kevent(kqueue_fd_, changes.data(), static_cast<int>(changes.size()),
             nullptr, 0, nullptr);
    }
  }

  int add_timer(std::chrono::milliseconds interval,
                std::function<void()> callback) override {
    int timer_id = next_timer_id_++;

    struct kevent ev;
    // NOTE_CRITICAL and NOTE_BACKGROUND are optional hints
    EV_SET(&ev, timer_id, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_USECONDS,
           interval.count() * 1000, nullptr);

    if (kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr) == -1) {
      throw std::runtime_error(
          std::format("kevent timer add failed: {}", std::strerror(errno)));
    }

    std::lock_guard lock(mutex_);
    timer_callbacks_[timer_id] = std::move(callback);

    return timer_id;
  }

  void cancel_timer(int timer_id) override {
    struct kevent ev;
    EV_SET(&ev, timer_id, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
    kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr);

    std::lock_guard lock(mutex_);
    timer_callbacks_.erase(timer_id);
  }

  int poll(int timeout_ms) override {
    std::array<struct kevent, 64> events{};

    struct timespec timeout;
    struct timespec* timeout_ptr = nullptr;

    if (timeout_ms >= 0) {
      timeout.tv_sec = timeout_ms / 1000;
      timeout.tv_nsec = (timeout_ms % 1000) * 1000000;
      timeout_ptr = &timeout;
    }

    int n = kevent(kqueue_fd_, nullptr, 0, events.data(),
                   static_cast<int>(events.size()), timeout_ptr);

    if (n == -1) {
      if (errno == EINTR) {
        return 0;
      }
      return -1;
    }

    for (int i = 0; i < n; ++i) {
      const struct kevent& ev = events[i];

      // Handle timer events
      if (ev.filter == EVFILT_TIMER) {
        std::function<void()> cb;
        {
          std::lock_guard lock(mutex_);
          auto it = timer_callbacks_.find(static_cast<int>(ev.ident));
          if (it != timer_callbacks_.end()) {
            cb = it->second;
          }
        }
        if (cb) {
          cb();
        }
        continue;
      }

      // Handle signal events
      if (ev.filter == EVFILT_SIGNAL) {
        std::function<void()> cb;
        {
          std::lock_guard lock(mutex_);
          auto it = signal_callbacks_.find(static_cast<int>(ev.ident));
          if (it != signal_callbacks_.end()) {
            cb = it->second;
          }
        }
        if (cb) {
          cb();
        }
        continue;
      }

      // Handle fd events
      int fd = static_cast<int>(ev.ident);
      std::function<void(std::uint32_t)> cb;
      {
        std::lock_guard lock(mutex_);
        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) {
          cb = it->second;
        }
      }

      if (cb) {
        std::uint32_t mask = 0;

        if (ev.filter == EVFILT_READ) {
          mask |= EventMask::Read;
          if (ev.flags & EV_EOF) {
            mask |= EventMask::HangUp;
          }
        }

        if (ev.filter == EVFILT_WRITE) {
          mask |= EventMask::Write;
        }

        if (ev.flags & EV_ERROR) {
          mask |= EventMask::Error;
        }

        cb(mask);
      }
    }

    return n;
  }

  void add_signal(int signum, std::function<void()> callback) override {
    // Block the signal so it doesn't invoke default handler
    signal(signum, SIG_IGN);

    struct kevent ev;
    EV_SET(&ev, signum, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, nullptr);

    if (kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr) == -1) {
      throw std::runtime_error(
          std::format("kevent signal add failed: {}", std::strerror(errno)));
    }

    std::lock_guard lock(mutex_);
    signal_callbacks_[signum] = std::move(callback);
  }

private:
  int kqueue_fd_{-1};
  int next_timer_id_{1};
  std::mutex mutex_;
  std::unordered_map<int, std::function<void(std::uint32_t)>> callbacks_;
  std::unordered_map<int, std::uint32_t> fd_events_;
  std::unordered_map<int, std::function<void()>> timer_callbacks_;
  std::unordered_map<int, std::function<void()>> signal_callbacks_;
};

std::unique_ptr<EventLoopBackend> create_event_loop_backend() {
  auto backend = std::make_unique<DarwinEventLoopBackend>();
  backend->init();
  return backend;
}

} // namespace neonsignal
