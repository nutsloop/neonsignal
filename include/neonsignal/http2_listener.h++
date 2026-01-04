#pragma once

#include "neonsignal/neonsignal.h++"
#include "neonsignal/database.h++"
#include "neonsignal/hpack_decoder.h++"
#include "neonsignal/vhost.h++"
#include "neonsignal/webauthn.h++"

#include <openssl/ssl.h>
#include <sys/epoll.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <fstream>
#include <unordered_map>
#include <thread>
#include <vector>

struct ssl_ctx_st;
struct ssl_st;

namespace neonsignal {

struct Http2Connection {
  int fd{-1};
  std::unique_ptr<SSL, decltype(&SSL_free)> ssl{nullptr, &SSL_free};
  bool handshake_complete{false};
  bool preface_ok{false};
  bool client_settings_seen{false};
  bool server_settings_sent{false};
  std::vector<std::uint8_t> read_buf;
  std::vector<std::uint8_t> write_buf;
  std::size_t write_offset{0};
  std::uint32_t events{EPOLLIN};
  bool closed{false};
  std::string last_path = "/";
  std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> header_blocks;
  std::chrono::steady_clock::time_point handshake_deadline{};
  bool first_header_logged{false};
  std::unique_ptr<HpackDecoder> decoder;

  // SSE stream tracking
  bool is_event_stream{false};
  std::uint32_t event_stream_id{0};
  std::uint64_t event_sse_count{0};
  std::chrono::steady_clock::time_point event_sse_start{};
  bool is_cpu_stream{false};
  std::uint32_t cpu_stream_id{0};
  std::uint64_t last_cpu_time_ns{0};
  std::chrono::steady_clock::time_point last_cpu_wall{};
  std::uint64_t cpu_sse_count{0};
  std::chrono::steady_clock::time_point cpu_sse_start{};
  bool is_mem_stream{false};
  std::uint32_t mem_stream_id{0};
  std::uint64_t last_rss_kb{0};
  std::uint64_t mem_sse_count{0};
  std::chrono::steady_clock::time_point mem_sse_start{};
  bool is_redirect_stream{false};
  std::uint32_t redirect_stream_id{0};
  std::uint64_t redirect_sse_count{0};
  std::chrono::steady_clock::time_point redirect_sse_start{};

  // Resource management and timeouts
  std::chrono::steady_clock::time_point created_at{std::chrono::steady_clock::now()};
  std::chrono::steady_clock::time_point last_activity{std::chrono::steady_clock::now()};
  bool has_write_backpressure{false};
  bool tls_handshake_offloaded{false};  // TLS handshake in thread pool

  // Session caching
  std::string cached_session_token;
  std::string cached_user_id;
  bool session_validated{false};

  struct StreamState {
    std::string path;
    std::string method;
    bool expect_body{false};
    bool responded{false};
    bool is_upload{false};
    bool is_auth_finish{false};
    bool is_register_finish{false};
    bool is_user_register{false};
    bool is_user_verify{false};
    bool is_user_enroll{false};
    std::uint64_t session_user_id{0};
    std::vector<std::uint8_t> body;
    std::string file_rel_path;
    std::string file_full_path;
    std::string upload_name;
    std::string reg_secret;
    std::ofstream file;
    std::uint64_t received_bytes{0};
    bool write_failed{false};
    std::string content_type;
    bool is_codex{false};
  };
  std::unordered_map<std::uint32_t, StreamState> streams;
};

} // namespace neonsignal

// New performance components (header-only) - included after Http2Connection definition
// Must be outside namespace to avoid polluting system header includes
#include "neonsignal/connection_manager.h++"
#include "neonsignal/static_cache.h++"
#include "neonsignal/session_cache.h++"
#include "neonsignal/sse_broadcaster.h++"

namespace neonsignal {

class EventLoop;
class ThreadPool;
class ApiHandler;

struct SSEResetPolicy {
  enum class Mode { OnlyTime, OnlyCount, Both };
  Mode mode{Mode::OnlyTime};
  std::chrono::seconds max_age{std::chrono::seconds(45)};
  std::uint64_t max_messages{200};
};

class Http2Listener {
public:
  Http2Listener(EventLoop& loop, ThreadPool& pool, SSL_CTX* ssl_ctx,
                ServerConfig config, const Router& router,
                std::atomic<std::uint64_t>& served_files,
                std::atomic<std::uint64_t>& page_views,
                std::atomic<std::uint64_t>& event_clients);
  ~Http2Listener();

  void start();
  void shutdown_graceful();

private:
  void setup_listener_();
  void handle_accept_();
  void handle_connection_(int client_fd);
  void register_connection_(std::shared_ptr<Http2Connection> conn);
  void handle_io_(const std::shared_ptr<Http2Connection>& conn,
                  std::uint32_t events);
  void close_connection_(int fd);
  void start_redirect_monitor_();
  void stop_redirect_monitor_();
  bool probe_redirect_service_();

  // New: Timeout and cleanup handlers
  void check_connection_timeouts_();
  void cleanup_expired_sessions_();
  void flush_sse_batches_();

  // New: TLS handshake offloading
  void offload_tls_handshake_(std::shared_ptr<Http2Connection> conn);
  void complete_tls_handshake_(std::shared_ptr<Http2Connection> conn, bool success);

  int listen_fd_{-1};
  EventLoop& loop_;
  ThreadPool& pool_;
  SSL_CTX* ssl_ctx_;
  ServerConfig config_;
  const Router& router_;
  std::atomic<std::uint64_t>& served_files_;
  std::atomic<std::uint64_t>& page_views_;
  std::atomic<std::uint64_t>& event_clients_;

  // New: Advanced resource management
  std::unique_ptr<ConnectionManager> conn_manager_;
  std::unique_ptr<StaticFileCache> static_cache_;
  std::unique_ptr<SessionCache> session_cache_;
  std::unique_ptr<SSEBroadcaster> sse_broadcaster_;
  std::unique_ptr<Database> db_;

  std::atomic<bool> redirect_service_ok_{false};
  int redirect_probe_port_{9090};
  int redirect_timer_fd_{-1};
  WebAuthnManager auth_;
  std::unique_ptr<ApiHandler> api_handler_;
  VHostResolver vhost_resolver_;

  SSEResetPolicy sse_policy_{SSEResetPolicy::Mode::OnlyTime, std::chrono::seconds(45), 200};
};

} // namespace neonsignal
