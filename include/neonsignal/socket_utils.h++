#pragma once

#include <cstdint>
#include <sys/socket.h>

namespace neonsignal {
namespace socket_utils {

// Set a file descriptor to non-blocking mode
// Returns 0 on success, -1 on error
int set_nonblocking(int fd);

// Set close-on-exec flag on a file descriptor
// Returns 0 on success, -1 on error
int set_cloexec(int fd);

// Accept a connection and set non-blocking + close-on-exec flags
// This is a portable replacement for accept4(SOCK_NONBLOCK | SOCK_CLOEXEC)
// Returns the new fd on success, -1 on error
int accept_nonblocking(int listen_fd, struct sockaddr* addr, socklen_t* addrlen);

// Create a socket with non-blocking and close-on-exec flags
// Portable replacement for socket() with SOCK_NONBLOCK | SOCK_CLOEXEC
int socket_nonblocking(int domain, int type, int protocol);

// Set SO_NOSIGPIPE on macOS (no-op on Linux where MSG_NOSIGNAL is used)
int set_nosigpipe(int fd);

// Get the appropriate send flags for the platform
// Returns MSG_NOSIGNAL on Linux, 0 on macOS (where SO_NOSIGPIPE is used)
int get_send_flags();

} // namespace socket_utils
} // namespace neonsignal
