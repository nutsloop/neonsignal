#pragma once

#include "neonsignal/codex_runner.h++"
#include "neonsignal/database.h++"
#include "neonsignal/http2_listener.h++"

#include <atomic>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace neonsignal {

class ApiHandler {
public:
  struct ApiResponse {
    int status{200};
    std::string content_type{"application/json"};
    std::vector<std::uint8_t> body;
    bool set_session_cookie{false};
    std::string session_id;
  };

  ApiHandler(EventLoop &loop, WebAuthnManager &auth, const Router &router, Database &db,
             std::atomic<std::uint64_t> &served_files, std::atomic<std::uint64_t> &page_views,
             std::atomic<std::uint64_t> &event_clients, std::atomic<bool> &redirect_ok);

  bool auth_login_options(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id);
  bool auth_login_finish_headers(const std::shared_ptr<Http2Connection> &conn,
                                 std::uint32_t stream_id, const std::string &path,
                                 const std::string &method);
  bool auth_user_check(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                       const std::unordered_map<std::string, std::string> &headers);
  bool user_register_headers(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                             const std::string &path, const std::string &method);
  ApiResponse user_register_finish(std::span<const std::uint8_t> payload);
  bool user_verify_headers(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                           const std::string &path, const std::string &method);
  ApiResponse user_verify_finish(std::span<const std::uint8_t> payload);
  bool user_enroll(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                   const std::unordered_map<std::string, std::string> &headers);
  bool user_enroll_headers(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                           const std::unordered_map<std::string, std::string> &headers,
                           const std::string &path, const std::string &method);
  ApiResponse user_enroll_finish(std::uint64_t user_id, std::span<const std::uint8_t> payload);
  bool codex_brief_headers(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                           const std::unordered_map<std::string, std::string> &headers,
                           const std::string &path, const std::string &method);
  ApiResponse codex_brief_finish(std::string_view content_type,
                                 std::span<const std::uint8_t> payload);
  bool codex_list(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id);
  bool codex_item(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                  const std::string &path);
  bool codex_image(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                   const std::string &path);
  bool codex_run_start(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                       const std::string &path, const std::string &method);
  bool codex_run_status(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                        const std::string &path);
  bool codex_run_stdout(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                        const std::string &path);
  bool codex_run_stderr(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                        const std::string &path);
  bool codex_run_artifacts(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                           const std::string &path);
  bool incoming_data(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                     const std::string &method, const std::string &upload_header_name,
                     const std::string &path);
  bool stats(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
             const std::string &path, const std::string &method, const std::string &authority);
  bool events(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
              const std::string &path, const std::string &method, const std::string &authority);
  bool cpu_stream(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                  const std::string &path, const std::string &method, const std::string &authority);
  bool memory_stream(const std::shared_ptr<Http2Connection> &conn, std::uint32_t stream_id,
                     const std::string &path, const std::string &method,
                     const std::string &authority);
  bool redirect_service_stream(const std::shared_ptr<Http2Connection> &conn,
                               std::uint32_t stream_id, const std::string &path,
                               const std::string &method, const std::string &authority);

private:
  EventLoop &loop_;
  WebAuthnManager &auth_;
  const Router &router_;
  Database &db_;
  std::atomic<std::uint64_t> &served_files_;
  std::atomic<std::uint64_t> &page_views_;
  std::atomic<std::uint64_t> &event_clients_;
  std::atomic<bool> &redirect_service_ok_;
  CodexRunner codex_runner_;
};

} // namespace neonsignal
