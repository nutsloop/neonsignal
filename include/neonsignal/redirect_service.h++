#pragma once

#include "neonsignal/event_loop.h++"

#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace neonsignal {

class RedirectService {
public:
  explicit RedirectService(int listen_port = 9090, std::string redirect_host = "0.0.0.0",
                           int redirect_port = 443, std::string acme_root = "acme-challenge");
  ~RedirectService();

  void start();
  void stop();

private:
  struct Connection {
    int fd{-1};
    std::string buffer;
    std::string write_buffer;
    bool write_ready{false};
    bool responded{false};
  };

  void setup_listener_();
  void handle_accept_();
  void register_connection_(int client_fd);
  void handle_io_(int fd, std::uint32_t events);
  void process_buffer_(int fd, Connection &conn);
  void send_redirect_(Connection &conn, const std::string &host, const std::string &path) const;
  bool serve_acme_challenge_(Connection &conn, const std::string &path) const;
  void close_connection_(int fd);

  EventLoop loop_;
  int listen_fd_{-1};
  int listen_port_{9090};
  std::string redirect_host_;
  int redirect_port_{443};
  std::string acme_root_;
  std::unordered_map<int, Connection> connections_;
  std::atomic<bool> running_{false};
};

} // namespace neonsignal
