#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/event_mask.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <iostream>
#include <sstream>
#include <string_view>

namespace neonsignal {

using namespace std::string_view_literals;

namespace {

std::string extract_cookie(const std::string& cookie_header, std::string_view name) {
  std::string target = std::string(name) + "=";
  auto pos = cookie_header.find(target);
  if (pos == std::string::npos) {
    return {};
  }
  pos += target.size();
  auto end = cookie_header.find(';', pos);
  if (end == std::string::npos) {
    return cookie_header.substr(pos);
  }
  return cookie_header.substr(pos, end - pos);
}

} // namespace

bool ApiHandler::user_enroll(const std::shared_ptr<Http2Connection>& conn,
                             std::uint32_t stream_id,
                             const std::unordered_map<std::string, std::string>& headers) {
  // GET request - return WebAuthn options for enrollment
  auto cookie_it = headers.find("cookie");
  std::string session_id;
  if (cookie_it != headers.end()) {
    session_id = extract_cookie(cookie_it->second, "ns_session");
  }

  if (session_id.empty()) {
    std::string body = "{\"error\":\"session required\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 401, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  // Validate session and check state
  auto session = db_.validate_session(session_id);
  if (!session) {
    std::string body = "{\"error\":\"invalid session\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 401, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  if (session->state != "pre_webauthn") {
    std::string body = "{\"error\":\"invalid session state\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 403, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  // Get user info
  auto user = db_.find_user_by_id(session->user_id);
  if (!user) {
    std::string body = "{\"error\":\"user not found\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 404, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  // Generate WebAuthn registration options
  auto opts = auth_.make_register_options_for_user(user->id, user->email, user->display_name);
  if (opts.json.empty()) {
    std::string body = "{\"error\":\"failed to generate options\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 500, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  std::vector<std::uint8_t> body_bytes(opts.json.begin(), opts.json.end());
  build_response_frames(conn->write_buf, stream_id, 200, "application/json", body_bytes);
  conn->events |= EventMask::Write;
  loop_.update_fd(conn->fd, conn->events);
  return true;
}

bool ApiHandler::user_enroll_headers(const std::shared_ptr<Http2Connection>& conn,
                                     std::uint32_t stream_id,
                                     const std::unordered_map<std::string, std::string>& headers,
                                     const std::string& path,
                                     const std::string& method) {
  // POST request - finish enrollment with WebAuthn attestation
  auto cookie_it = headers.find("cookie");
  std::string session_id;
  if (cookie_it != headers.end()) {
    session_id = extract_cookie(cookie_it->second, "ns_session");
  }

  if (session_id.empty()) {
    std::string body = "{\"error\":\"session required\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 401, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  // Validate session and check state
  auto session = db_.validate_session(session_id);
  if (!session) {
    std::string body = "{\"error\":\"invalid session\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 401, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  if (session->state != "pre_webauthn") {
    std::string body = "{\"error\":\"invalid session state\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 403, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  // Store session info for body processing
  Http2Connection::StreamState st{};
  st.path = path;
  st.method = method;
  st.expect_body = true;
  st.is_user_enroll = true;
  st.session_user_id = session->user_id;
  conn->streams[stream_id] = std::move(st);
  return true;
}

ApiHandler::ApiResponse ApiHandler::user_enroll_finish(std::uint64_t user_id,
                                                       std::span<const std::uint8_t> payload) {
  ApiResponse res;
  res.status = 400;
  res.content_type = "application/json";

  std::string body_str(payload.begin(), payload.end());

  // Finish WebAuthn registration
  auto result = auth_.finish_register_for_user(body_str, user_id);
  if (!result.ok) {
    std::ostringstream out;
    out << "{\"error\":\"" << result.error << "\"}";
    std::string resp = out.str();
    res.body = std::vector<std::uint8_t>(resp.begin(), resp.end());
    return res;
  }

  // Get user for session upgrade
  auto user = db_.find_user_by_id(user_id);
  if (!user) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"user not found\"}"sv.begin(),
        "{\"error\":\"user not found\"}"sv.end());
    return res;
  }

  // Issue new auth session (5 day TTL)
  std::string session_id = auth_.issue_session(user_id, user->email, "auth");
  if (session_id.empty()) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"failed to create session\"}"sv.begin(),
        "{\"error\":\"failed to create session\"}"sv.end());
    return res;
  }

  std::cerr << "[ENROLL] User " << user->email << " enrolled WebAuthn credential\n";

  res.status = 200;
  std::ostringstream out;
  out << "{\"ok\":true,\"session_id\":\"" << session_id << "\"}";
  std::string resp = out.str();
  res.body = std::vector<std::uint8_t>(resp.begin(), resp.end());
  return res;
}

} // namespace neonsignal
