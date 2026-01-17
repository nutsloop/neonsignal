#include "neonsignal/redirect_service.h++"
#include "neonsignal/socket_utils.h++"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>

namespace neonsignal {

void RedirectService::handle_accept_() {
  while (true) {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    const int client_fd = socket_utils::accept_nonblocking(
        listen_fd_, reinterpret_cast<sockaddr *>(&client_addr), &len);
    if (client_fd == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      break;
    }

    register_connection_(client_fd);
  }
}

} // namespace neonsignal
