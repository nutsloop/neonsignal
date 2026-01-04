#include "neonsignal/redirect_service.h++"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

namespace neonsignal {

void RedirectService::setup_listener_() {
  listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (listen_fd_ == -1) {
    throw std::runtime_error("redirect: failed to create socket");
  }

  const int enable = 1;
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
#ifdef SO_REUSEPORT
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
#endif

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<uint16_t>(listen_port_));
  if (redirect_host_ == "0.0.0.0") {
    addr.sin_addr.s_addr = INADDR_ANY;
  } else {
    if (inet_pton(AF_INET, redirect_host_.c_str(), &addr.sin_addr) != 1) {
      close(listen_fd_);
      listen_fd_ = -1;
      throw std::runtime_error("redirect: invalid host " + redirect_host_);
    }
  }

  if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1) {
    close(listen_fd_);
    listen_fd_ = -1;
    throw std::runtime_error("redirect: bind failed");
  }

  if (listen(listen_fd_, 128) == -1) {
    close(listen_fd_);
    listen_fd_ = -1;
    throw std::runtime_error("redirect: listen failed");
  }

  loop_.add_fd(listen_fd_, EPOLLIN, [this](std::uint32_t /*events*/) { handle_accept_(); });
}

} // namespace neonsignal
