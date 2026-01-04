#include "neonsignal/http2_listener.h++"
#include "neonsignal/thread_pool.h++"

#include "neonsignal/http2_listener_helpers.h++"

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

namespace neonsignal {

void Http2Listener::handle_accept_() {
  for (;;) {
    int client_fd = accept4(listen_fd_, nullptr, nullptr,
                            SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      if (errno == EINTR) {
        continue;
      }
      std::cerr << "accept failed: " << strerror(errno) << '\n';
      break;
    }

    // Check connection limit (DoS protection)
    if (!conn_manager_->can_accept_connection()) {
      std::cerr << "Connection limit reached, rejecting fd=" << client_fd << '\n';
      close(client_fd);
      continue;
    }

    pool_.enqueue([this, client_fd] { handle_connection_(client_fd); });
  }
}

} // namespace neonsignal
