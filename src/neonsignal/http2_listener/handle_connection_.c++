#include "neonsignal/http2_listener.h++"

#include "neonsignal/http2_listener_helpers.h++"

#include <fcntl.h>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <memory>

namespace neonsignal {

void Http2Listener::handle_connection_(int client_fd) {
  int flags = fcntl(client_fd, F_GETFL, 0);
  fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

  SSL *ssl = SSL_new(ssl_ctx_);
  if (!ssl) {
    close(client_fd);
    return;
  }

  auto conn = std::make_shared<Http2Connection>();
  conn->fd = client_fd;
  conn->ssl.reset(ssl);

  SSL_set_fd(ssl, client_fd);
  SSL_set_accept_state(ssl);
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_ENABLE_PARTIAL_WRITE);

  conn->handshake_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  conn->events = EPOLLIN | EPOLLOUT;
  conn->decoder = std::make_unique<HpackDecoder>();

  std::cerr << "Accepted HTTP/2-capable TLS connection fd=" << client_fd << '\n';
  register_connection_(std::move(conn));
}

} // namespace neonsignal
