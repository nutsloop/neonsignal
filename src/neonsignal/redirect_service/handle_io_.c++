#include "neonsignal/redirect_service.h++"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>

namespace neonsignal {

void RedirectService::handle_io_(const int fd, const std::uint32_t events) {

  const auto it = connections_.find(fd);
  if (it == connections_.end()) {
    std::cerr << "redirect: missing connection state fd=" << fd << '\n';
    return;
  }
  Connection &conn = it->second;

  if (events & (EPOLLERR | EPOLLHUP)) {
    // Peer hung up or errored before we finished; drop the connection.
    std::cerr << "redirect: epoll err/hup fd=" << fd << '\n';
    close_connection_(fd);
    return;
  }

  if (events & EPOLLIN) {
    char buf[2048];
    while (true) {
      // Read as much as available; the redirect response is tiny so we only
      // need the request headers.

      if (const ssize_t n = recv(fd, buf, sizeof(buf), 0); n > 0) {
        std::cerr << "redirect: reading headers fd=" << fd << '\n';
        conn.buffer.append(buf, static_cast<std::size_t>(n));

        if (conn.buffer.size() > 32768) {
          // Malformed or too large for a redirect-only service.
          std::cerr << "redirect: header buffer too large fd=" << fd << '\n';
          break;
        }
      } else if (n == 0) {
        // Client closed.
        if (!conn.buffer.empty()) {
          std::cerr << "redirect: client closed fd=" << fd << '\n';
        }
        close_connection_(fd);
        return;
      } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          break;
        }
        // Hard read error.
        std::cerr << "redirect: recv error fd=" << fd << " errno=" << errno << '\n';
        close_connection_(fd);
        return;
      }
    }
    // If headers arrived, parse and enqueue the redirect payload.
    process_buffer_(fd, conn);
  }

  if ((events & EPOLLOUT) && conn.write_ready) {
    while (!conn.write_buffer.empty()) {
      // Send until the buffer empties or the socket would block; MSG_NOSIGNAL
      // avoids SIGPIPE if the peer vanished.
      const ssize_t n = send(fd, conn.write_buffer.data(), conn.write_buffer.size(), MSG_NOSIGNAL);
      if (n > 0) {
        conn.write_buffer.erase(0, static_cast<std::size_t>(n));
        continue;
      }
      if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        // Wait for next writable edge-trigger event.
        break;
      }
      // Fatal write error.
      std::cerr << "redirect: send error fd=" << fd << " errno=" << errno << '\n';
      close_connection_(fd);
      return;
    }

    if (conn.write_buffer.empty()) {
      // After flushing the redirect, close to keep things simple.
      std::cerr << "redirect: response flushed fd=" << fd << ", closing\n";
      close_connection_(fd);
    }
  }
}

} // namespace neonsignal
