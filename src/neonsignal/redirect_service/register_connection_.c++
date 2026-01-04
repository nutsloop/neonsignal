#include "neonsignal/redirect_service.h++"

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace neonsignal {

void RedirectService::register_connection_(int client_fd) {
  const int flags = fcntl(client_fd, F_GETFL, 0);
  fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

#ifdef SO_NOSIGPIPE
  int one = 1;
  setsockopt(client_fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif

  Connection conn{};
  conn.fd = client_fd;
  connections_.emplace(client_fd, std::move(conn));

  loop_.add_fd(client_fd, EPOLLIN | EPOLLET,
               [this, client_fd](const std::uint32_t events) { handle_io_(client_fd, events); });
}

} // namespace neonsignal
