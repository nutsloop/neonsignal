#include "spin/socket_utils.h++"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/types.h>
#endif

namespace neonsignal {
namespace socket_utils {

int set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    return -1;
  }
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int set_cloexec(int fd) {
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags == -1) {
    return -1;
  }
  return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

int accept_nonblocking(int listen_fd, struct sockaddr* addr, socklen_t* addrlen) {
#ifdef __linux__
  // Linux supports accept4 with flags
  int fd = accept4(listen_fd, addr, addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  return fd;
#else
  // macOS/BSD: use accept + fcntl
  int fd = accept(listen_fd, addr, addrlen);
  if (fd == -1) {
    return -1;
  }

  if (set_nonblocking(fd) == -1 || set_cloexec(fd) == -1) {
    close(fd);
    return -1;
  }

  // Set SO_NOSIGPIPE on macOS to prevent SIGPIPE on write to closed socket
  if (set_nosigpipe(fd) == -1) {
    close(fd);
    return -1;
  }

  return fd;
#endif
}

int socket_nonblocking(int domain, int type, int protocol) {
#ifdef __linux__
  // Linux supports SOCK_NONBLOCK and SOCK_CLOEXEC flags directly
  return socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
#else
  // macOS/BSD: create socket then set flags
  int fd = socket(domain, type, protocol);
  if (fd == -1) {
    return -1;
  }

  if (set_nonblocking(fd) == -1 || set_cloexec(fd) == -1) {
    close(fd);
    return -1;
  }

  return fd;
#endif
}

int set_nosigpipe([[maybe_unused]] int fd) {
#ifdef __APPLE__
  int optval = 1;
  return setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#else
  // Linux uses MSG_NOSIGNAL on send() instead
  return 0;
#endif
}

int get_send_flags() {
#ifdef __linux__
  return MSG_NOSIGNAL;
#else
  // macOS uses SO_NOSIGPIPE socket option instead
  return 0;
#endif
}

} // namespace socket_utils
} // namespace neonsignal
