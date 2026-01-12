#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>

namespace neonsignal {

class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  void add_fd(int fd, std::uint32_t events, std::function<void(std::uint32_t)> callback);
  void update_fd(int fd, std::uint32_t events);
  void remove_fd(int fd);

  void run();
  void stop();

  // Graceful shutdown - drain connections with timeout
  void shutdown_graceful(std::chrono::seconds timeout = std::chrono::seconds{30});

  // Register a periodic timer callback
  void add_timer(std::chrono::milliseconds interval, std::function<void()> callback);

  // Check if running
  [[nodiscard]] bool is_running() const { return running_.load(std::memory_order_relaxed); }

  // Get active FD count
  [[nodiscard]] std::size_t active_fd_count() const {
    std::lock_guard lock(callbacks_mutex_);
    return callbacks_.size();
  }

private:
  int epoll_fd_;
  std::atomic<bool> running_{false};
  std::atomic<bool> shutdown_requested_{false};
  mutable std::mutex callbacks_mutex_; // mutable to allow locking in const methods
  std::unordered_map<int, std::function<void(std::uint32_t)>> callbacks_;
  int timer_fd_{-1};
  std::function<void()> timer_callback_;
};

} // namespace neonsignal
