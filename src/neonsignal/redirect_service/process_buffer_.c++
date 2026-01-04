#include "neonsignal/redirect_service.h++"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace neonsignal {

void RedirectService::process_buffer_(const int fd, Connection &conn) {
  if (conn.responded) {
    std::cerr << "redirect: already responded fd=" << fd << '\n';
    // Redirect already queued; avoid double responses.
    return;
  }

  if (const auto header_end = conn.buffer.find("\r\n\r\n"); header_end == std::string::npos) {
    std::cerr << "redirect: waiting for full headers fd=" << fd << '\n';
    // Wait for complete headers.
    return;
  }

  std::string host = redirect_host_;
  std::string path = "/";
  std::string method = "GET";
  std::uint16_t host_port = redirect_port_;
  std::uint16_t client_port = 0;

  // Best-effort capture of client port for logging/observability.
  sockaddr_in peer{};
  socklen_t peer_len = sizeof(peer);
  if (getpeername(fd, reinterpret_cast<sockaddr *>(&peer), &peer_len) == 0 &&
      peer.sin_family == AF_INET) {
    client_port = ntohs(peer.sin_port);
  }

  // Parse request line: METHOD SP PATH SP VERSION
  if (const auto line_end = conn.buffer.find("\r\n"); line_end != std::string::npos) {

    std::string request_line = conn.buffer.substr(0, line_end);
    const auto first_space = request_line.find(' ');
    const auto second_space = first_space == std::string::npos
                                  ? std::string::npos
                                  : request_line.find(' ', first_space + 1);
    if (first_space != std::string::npos && second_space != std::string::npos &&
        second_space > first_space + 1) {
      method = request_line.substr(0, first_space);
      path = request_line.substr(first_space + 1, second_space - first_space - 1);
      if (path.empty() || path.front() != '/') {
        // Normalize odd paths back to root.
        path = "/";
      }
      std::cerr << "redirect: parsed path fd=" << fd << " path=" << path << " method=" << method
                << '\n';
    }
  }

  // Extract Host header to preserve the hostname in the Location target.
  if (const auto host_pos = conn.buffer.find("Host:"); host_pos != std::string::npos) {

    if (auto const host_line_end = conn.buffer.find("\r\n", host_pos);
        host_line_end != std::string::npos) {

      auto start = host_pos + 5;
      while (start < host_line_end && conn.buffer[start] == ' ') {
        ++start;
      }
      if (start < host_line_end) {
        std::string header_host = conn.buffer.substr(start, host_line_end - start);
        header_host.erase(header_host.begin(),
                          std::ranges::find_if(
                              header_host, [](const unsigned char c) { return !std::isspace(c); }));
        while (!header_host.empty() && std::isspace(header_host.back())) {
          header_host.pop_back();
        }
        if (!header_host.empty()) {
          if (auto const colon = header_host.find(':'); colon != std::string::npos) {
            std::string const port_str = header_host.substr(colon + 1);
            try {
              if (int const p = std::stoi(port_str); p > 0 && p <= 65535) {
                host_port = static_cast<std::uint16_t>(p);
              }
            } catch (...) {
            }
            header_host = header_host.substr(0, colon);
          }
          // Use the client-sent host when present.
          host = header_host;
          std::cerr << "redirect: parsed host fd=" << fd << " host=" << host
                    << " port=" << host_port << '\n';
        }
      }
    }
  }

  if (path.rfind("/.well-known/acme-challenge/", 0) == 0) {
    if (const bool ok = serve_acme_challenge_(conn, path); !ok) {

      conn.write_buffer =
          "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
      conn.write_ready = true;
      std::cerr << "redirect: ACME 404 fd=" << fd << " path=" << path << '\n';
    } else {
      std::cerr << "redirect: ACME served fd=" << fd << " path=" << path << '\n';
    }
    conn.responded = true;
    loop_.update_fd(fd, EPOLLOUT | EPOLLET);
    return;
  }

  conn.responded = true;
  // Queue the redirect and flip to EPOLLOUT to flush.
  send_redirect_(conn, host, path);
  std::cerr << "redirect: queued 308 fd=" << fd << " host=" << host << " port=" << host_port
            << " method=" << method << " path=" << path << " client_port=" << client_port << '\n';
  loop_.update_fd(fd, EPOLLOUT | EPOLLET);
}

} // namespace neonsignal
