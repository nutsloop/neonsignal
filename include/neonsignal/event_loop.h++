#pragma once

#include "neonsignal/event_loop_backend.h++"
#include "neonsignal/event_mask.h++"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace neonsignal {

class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  // Non-copyable, non-movable
  EventLoop(const EventLoop &) = delete;
  EventLoop &operator=(const EventLoop &) = delete;
  EventLoop(EventLoop &&) = delete;
  EventLoop &operator=(EventLoop &&) = delete;

  void add_fd(int fd, std::uint32_t events, std::function<void(std::uint32_t)> callback);
  void update_fd(int fd, std::uint32_t events);
  void remove_fd(int fd);

  void run();
  void stop();

  // Graceful shutdown - drain connections with timeout
  void shutdown_graceful(std::chrono::seconds timeout = std::chrono::seconds{3});

  // Register a periodic timer callback
  int add_timer(std::chrono::milliseconds interval, std::function<void()> callback);

  // Cancel a timer by id
  void cancel_timer(int timer_id);

  // Add signal handler for graceful shutdown
  void add_signal(int signum, std::function<void()> callback);

  // Check if running
  [[nodiscard]] bool is_running() const { return running_.load(std::memory_order_relaxed); }

  // Get active FD count
  [[nodiscard]] std::size_t active_fd_count() const {
    std::lock_guard lock(callbacks_mutex_);
    return callbacks_.size();
  }

private:
  std::unique_ptr<EventLoopBackend> backend_;
  std::atomic<bool> running_{false};
  std::atomic<bool> shutdown_requested_{false};
  mutable std::mutex callbacks_mutex_;
  std::unordered_map<int, std::function<void(std::uint32_t)>> callbacks_;
};

} // namespace neonsignal
