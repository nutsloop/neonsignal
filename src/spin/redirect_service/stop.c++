#include "spin/redirect_service.h++"

#include <ranges>
#include <unistd.h>

namespace neonsignal {

void RedirectService::stop() {
  if (!running_.exchange(false)) {
    return;
  }

  loop_.stop();

  if (listen_fd_ != -1) {
    loop_.remove_fd(listen_fd_);
    close(listen_fd_);
    listen_fd_ = -1;
  }

  for (const auto &val : connections_ | std::views::values) {
    close(val.fd);
  }
  connections_.clear();
}

} // namespace neonsignal
