#include "spin/event_loop.h++"
#include "spin/redirect_service.h++"

#include <unistd.h>

#include <iostream>

namespace neonsignal {

void RedirectService::close_connection_(const int fd) {
  // Stop watching the descriptor.
  loop_.remove_fd(fd);
  if (const auto it = connections_.find(fd); it != connections_.end()) {
    // Known connection: close the tracked socket and erase its state.
    if (!it->second.buffer.empty()) {
      std::cerr << "redirect: closing tracked fd=" << it->second.fd << '\n';
    }
    close(it->second.fd);
    connections_.erase(it);
  } else {
    // Safety: close untracked fd to avoid leaks.
    std::cerr << "redirect: closing untracked fd=" << fd << '\n';
    close(fd);
  }
}

} // namespace neonsignal
