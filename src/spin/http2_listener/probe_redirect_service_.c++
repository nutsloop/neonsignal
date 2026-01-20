#include "spin/http2_listener.h++"
#include "spin/socket_utils.h++"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace neonsignal {

bool Http2Listener::probe_redirect_service_() {
  int fd = socket_utils::socket_nonblocking(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    return false;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<uint16_t>(redirect_probe_port_));
  if (inet_pton(AF_INET, config_.host.c_str(), &addr.sin_addr) != 1) {
    close(fd);
    return false;
  }

  int ret = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  if (ret == -1 && errno != EINPROGRESS) {
    close(fd);
    return false;
  }

  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(fd, &wfds);
  timeval tv{};
  tv.tv_sec = 0;
  tv.tv_usec = 200000; // 200ms
  ret = select(fd + 1, nullptr, &wfds, nullptr, &tv);
  if (ret <= 0) {
    close(fd);
    return false;
  }

  int err = 0;
  socklen_t len = sizeof(err);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) == -1) {
    close(fd);
    return false;
  }
  close(fd);
  return err == 0;
}

} // namespace neonsignal
