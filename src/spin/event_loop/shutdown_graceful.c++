#include "spin/event_loop.h++"

#include <iostream>
#include <format>

namespace neonsignal {

void EventLoop::shutdown_graceful(std::chrono::seconds timeout) {
  std::cerr << std::format("▸ Graceful shutdown initiated (timeout: {}s)\n",
                           timeout.count());

  shutdown_requested_.store(true, std::memory_order_release);

  auto deadline = std::chrono::steady_clock::now() + timeout;

  // Continue processing events until all FDs are closed or timeout
  while (active_fd_count() > 0 &&
         std::chrono::steady_clock::now() < deadline) {

    // Process events with 1 second timeout
    backend_->poll(1000);

    std::cerr << std::format("• Draining connections... {} remaining\n",
                             active_fd_count());
  }

  if (active_fd_count() > 0) {
    std::cerr << std::format("▲ Timeout reached, {} connections force-closed\n",
                             active_fd_count());
  } else {
    std::cerr << "✓ All connections drained\n";
  }

  stop();
}

} // namespace neonsignal
