#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string_view>

namespace neonsignal {

using namespace std::string_view_literals;

namespace {

std::vector<std::uint8_t> base64url_decode(std::string_view input) {
  std::string s(input);
  std::replace(s.begin(), s.end(), '-', '+');
  std::replace(s.begin(), s.end(), '_', '/');
  while (s.size() % 4 != 0) {
    s.push_back('=');
  }
  int pad = 0;
  if (!s.empty() && s[s.size() - 1] == '=') ++pad;
  if (s.size() > 1 && s[s.size() - 2] == '=') ++pad;
  std::vector<unsigned char> buf((s.size() / 4) * 3 + 1);
  int len = EVP_DecodeBlock(buf.data(), reinterpret_cast<const unsigned char *>(s.data()),
                            static_cast<int>(s.size()));
  if (len < 0) {
    return {};
  }
  if (pad > 0 && len >= pad) {
    len -= pad;
  }
  return std::vector<std::uint8_t>(buf.begin(), buf.begin() + len);
}

std::string extract_json_string(std::string_view json, std::string_view key) {
  auto pos = json.find("\"" + std::string(key) + "\"");
  if (pos == std::string_view::npos) {
    return {};
  }
  pos = json.find(':', pos);
  if (pos == std::string_view::npos) {
    return {};
  }
  pos = json.find('"', pos);
  if (pos == std::string_view::npos) {
    return {};
  }
  std::size_t end = json.find('"', pos + 1);
  if (end == std::string_view::npos) {
    return {};
  }
  return std::string(json.substr(pos + 1, end - pos - 1));
}

} // namespace

bool ApiHandler::user_verify_headers(const std::shared_ptr<Http2Connection> &conn,
                                     std::uint32_t stream_id, const std::string &path,
                                     const std::string &method) {
  if (method != "POST") {
    std::string body = "{\"error\":\"method not allowed\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 405, "application/json", body_bytes);
    conn->events |= EPOLLOUT;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  Http2Connection::StreamState st{};
  st.path = path;
  st.method = method;
  st.expect_body = true;
  st.is_user_verify = true;
  conn->streams[stream_id] = std::move(st);
  return true;
}

ApiHandler::ApiResponse ApiHandler::user_verify_finish(std::span<const std::uint8_t> payload) {
  ApiResponse res;
  res.status = 400;
  res.content_type = "application/json";

  std::string body_str(payload.begin(), payload.end());
  std::string token_b64 = extract_json_string(body_str, "token");
  std::string email = extract_json_string(body_str, "email");

  if (email.empty()) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"email required\"}"sv.begin(),
                                         "{\"error\":\"email required\"}"sv.end());
    return res;
  }

  // If token is empty, check if user is already verified and issue session
  if (token_b64.empty()) {
    auto user = db_.find_user_by_email(email);
    if (!user) {
      res.body = std::vector<std::uint8_t>("{\"error\":\"user not found\"}"sv.begin(),
                                           "{\"error\":\"user not found\"}"sv.end());
      return res;
    }

    if (!user->verified) {
      res.body = std::vector<std::uint8_t>("{\"error\":\"account not verified\"}"sv.begin(),
                                           "{\"error\":\"account not verified\"}"sv.end());
      return res;
    }

    // User is verified, issue pre_webauthn session
    std::string session_id = auth_.issue_session(user->id, user->email, "pre_webauthn");
    if (session_id.empty()) {
      res.status = 500;
      res.body = std::vector<std::uint8_t>("{\"error\":\"failed to create session\"}"sv.begin(),
                                           "{\"error\":\"failed to create session\"}"sv.end());
      return res;
    }

    std::cerr << "[VERIFY] User " << email << " already verified, pre_webauthn session created\n";

    res.status = 200;
    res.set_session_cookie = true;
    res.session_id = session_id;
    std::ostringstream out;
    out << "{\"ok\":true,\"session_id\":\"" << session_id << "\"}";
    std::string resp = out.str();
    res.body = std::vector<std::uint8_t>(resp.begin(), resp.end());
    return res;
  }

  // Decode token
  auto token = base64url_decode(token_b64);
  if (token.size() != 32) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"invalid token format\"}"sv.begin(),
                                         "{\"error\":\"invalid token format\"}"sv.end());
    return res;
  }

  // Hash token for lookup
  std::vector<std::uint8_t> token_hash(SHA256_DIGEST_LENGTH);
  SHA256(token.data(), token.size(), token_hash.data());

  // Find verification record
  auto verification = db_.find_verification(token_hash);
  if (!verification) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"invalid or expired token\"}"sv.begin(),
                                         "{\"error\":\"invalid or expired token\"}"sv.end());
    return res;
  }

  // Check if already used
  if (verification->used_at != 0) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"token already used\"}"sv.begin(),
                                         "{\"error\":\"token already used\"}"sv.end());
    return res;
  }

  // Find user and verify email matches
  auto user = db_.find_user_by_id(verification->user_id);
  if (!user) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"user not found\"}"sv.begin(),
                                         "{\"error\":\"user not found\"}"sv.end());
    return res;
  }

  if (user->email != email) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"email mismatch\"}"sv.begin(),
                                         "{\"error\":\"email mismatch\"}"sv.end());
    return res;
  }

  // Mark verification as used
  if (!db_.mark_verification_used(token_hash)) {
    res.status = 500;
    res.body =
        std::vector<std::uint8_t>("{\"error\":\"failed to mark verification used\"}"sv.begin(),
                                  "{\"error\":\"failed to mark verification used\"}"sv.end());
    return res;
  }

  // Mark user as verified
  if (!db_.set_user_verified(user->id)) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>("{\"error\":\"failed to verify user\"}"sv.begin(),
                                         "{\"error\":\"failed to verify user\"}"sv.end());
    return res;
  }

  // Issue pre_webauthn session (5 minute TTL)
  std::string session_id = auth_.issue_session(user->id, user->email, "pre_webauthn");
  if (session_id.empty()) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>("{\"error\":\"failed to create session\"}"sv.begin(),
                                         "{\"error\":\"failed to create session\"}"sv.end());
    return res;
  }

  std::cerr << "[VERIFY] User " << email << " verified, pre_webauthn session created\n";

  res.status = 200;
  std::ostringstream out;
  out << "{\"ok\":true,\"session_id\":\"" << session_id << "\"}";
  std::string resp = out.str();
  res.body = std::vector<std::uint8_t>(resp.begin(), resp.end());
  return res;
}

} // namespace neonsignal
