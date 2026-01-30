#pragma once

#include "spin/mail_config.h++"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include <openssl/ssl.h>

namespace neonsignal {

class EventLoop;
class ThreadPool;
class Http2Listener;
class Router;
class CertManager;

struct ServerConfig {
  std::string host = "0.0.0.0";
  std::uint16_t port = 9443;
  std::string certs_root = "certs";
  std::string www_root = "public";
  std::string rp_id = "neonsignal.nutsloop.host";
  std::string origin = "https://neonsignal.nutsloop.host";
  std::string db_path = "data/neonsignal.mdb";
  std::string working_dir;
  MailConfig mail;
};

class Server {
public:
  explicit Server(ServerConfig config = {});
  ~Server();

  void initialize_tls();
  void configure_credentials();
  void run();
  void stop();

private:
  struct SSLContextDeleter {
    void operator()(SSL_CTX* ctx) const;
  };

  ServerConfig config_;
  std::unique_ptr<CertManager> cert_manager_;
  std::unique_ptr<SSL_CTX, SSLContextDeleter> ssl_ctx_;
  std::unique_ptr<EventLoop> loop_;
  std::unique_ptr<ThreadPool> pool_;
  std::unique_ptr<Http2Listener> listener_;
  std::unique_ptr<Router> router_;
  std::atomic<std::uint64_t> served_files_{0};
  std::atomic<std::uint64_t> page_views_{0};
  std::atomic<std::uint64_t> event_clients_{0};
};

} // namespace neonsignal
