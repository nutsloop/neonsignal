#include "spin/http2_listener.h++"

#include "spin/api_handler.h++"
#include "spin/event_loop.h++"
#include "spin/http2_listener_helpers.h++"
#include "spin/mail_cookie_store.h++"
#include "spin/mail_service.h++"

#include <csignal>
#include <stdexcept>
#include <unistd.h>

namespace neonsignal {

Http2Listener::Http2Listener(EventLoop& loop, ThreadPool& pool, SSL_CTX* ssl_ctx,
                             ServerConfig config, const Router& router,
                             std::atomic<std::uint64_t>& served_files,
                             std::atomic<std::uint64_t>& page_views,
                             std::atomic<std::uint64_t>& event_clients)
    : loop_(loop), pool_(pool), ssl_ctx_(ssl_ctx), config_(std::move(config)),
      router_(router), served_files_(served_files), page_views_(page_views),
      event_clients_(event_clients),
      conn_manager_(std::make_unique<ConnectionManager>()),
      static_cache_(std::make_unique<StaticFileCache>()),
      session_cache_(std::make_unique<SessionCache>()),
      sse_broadcaster_(std::make_unique<SSEBroadcaster>()),
      db_(std::make_unique<Database>(config_.db_path)),
      mail_service_(std::make_unique<MailService>(*db_, config_.mail)),
      mail_cookie_store_(std::make_unique<MailCookieStore>(config_.mail)),
      auth_(config_.rp_id, config_.origin, *db_),
      api_handler_(std::make_unique<ApiHandler>(loop_, auth_, router_, *db_,
                                                served_files_, page_views_, event_clients_,
                                                redirect_service_ok_,
                                                *mail_service_, *mail_cookie_store_,
                                                config_.mail)),
      vhost_resolver_(config_.www_root) {
  if (!ssl_ctx_) {
    throw std::runtime_error("Http2Listener requires a valid SSL_CTX");
  }
  auth_.load_credentials();
  signal(SIGPIPE, SIG_IGN);
}

Http2Listener::~Http2Listener() {
  stop_redirect_monitor_();
  if (listen_fd_ != -1) {
    loop_.remove_fd(listen_fd_);
    close(listen_fd_);
  }

  // ConnectionManager cleanup happens automatically via unique_ptr destructor
  // No need to manually iterate and close connections
}

} // namespace neonsignal
