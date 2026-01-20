#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

namespace neonsignal {

// Abstract backend interface for event loop implementations
// Linux uses epoll, macOS/BSD uses kqueue
class EventLoopBackend {
public:
  virtual ~EventLoopBackend() = default;

  // Initialize the backend (create epoll fd or kqueue)
  virtual void init() = 0;

  // Cleanup (close fds)
  virtual void cleanup() = 0;

  // Add a file descriptor with event mask and callback
  virtual void add_fd(int fd, std::uint32_t events,
                      std::function<void(std::uint32_t)> callback) = 0;

  // Update events for an existing fd
  virtual void update_fd(int fd, std::uint32_t events) = 0;

  // Remove a file descriptor
  virtual void remove_fd(int fd) = 0;

  // Add a periodic timer
  // Returns a timer id that can be used to cancel
  virtual int add_timer(std::chrono::milliseconds interval,
                        std::function<void()> callback) = 0;

  // Cancel a timer by id
  virtual void cancel_timer(int timer_id) = 0;

  // Poll for events and dispatch callbacks
  // Returns number of events processed, -1 on error
  // timeout_ms: -1 for infinite, 0 for non-blocking, >0 for timeout
  virtual int poll(int timeout_ms) = 0;

  // Add signal handler (for graceful shutdown)
  // Linux: signalfd, macOS: EVFILT_SIGNAL or self-pipe
  virtual void add_signal(int signum, std::function<void()> callback) = 0;
};

// Factory function - implemented per platform
std::unique_ptr<EventLoopBackend> create_event_loop_backend();

} // namespace neonsignal
