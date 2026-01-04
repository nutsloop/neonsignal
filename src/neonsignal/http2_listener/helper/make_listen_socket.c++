#include "neonsignal/http2_listener_helpers.h++"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace neonsignal {

int make_listen_socket(const ServerConfig& config) {
  int fd =
      ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (fd == -1) {
    throw std::runtime_error("failed to create socket");
  }

  int yes = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#ifdef SO_REUSEPORT
  setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
#endif

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(config.port);
  if (inet_pton(AF_INET, config.host.c_str(), &addr.sin_addr) != 1) {
    close(fd);
    throw std::runtime_error("invalid listen address: " + config.host);
  }

  if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
    close(fd);
    throw std::runtime_error("bind failed: " + std::string(strerror(errno)));
  }

  if (listen(fd, SOMAXCONN) == -1) {
    close(fd);
    throw std::runtime_error("listen failed: " + std::string(strerror(errno)));
  }

  return fd;
}

} // namespace neonsignal
