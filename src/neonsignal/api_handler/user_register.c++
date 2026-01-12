#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <openssl/rand.h>
#include <openssl/sha.h>

#include <iostream>
#include <sstream>
#include <string_view>

namespace neonsignal {

using namespace std::string_view_literals;

namespace {

std::string base64url_encode(const std::vector<std::uint8_t> &data) {
  static const char *kTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
  std::string out;
  out.reserve(((data.size() + 2) / 3) * 4);
  std::size_t i = 0;
  while (i + 2 < data.size()) {
    std::uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
    out.push_back(kTable[n & 63]);
    i += 3;
  }
  if (i + 1 == data.size()) {
    std::uint32_t n = (data[i] << 16);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
  } else if (i + 2 == data.size()) {
    std::uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
  }
  return out;
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

std::string json_escape(std::string_view value) {
  std::string out;
  out.reserve(value.size());
  for (char c : value) {
    switch (c) {
    case '\\':
    case '"':
      out.push_back('\\');
      out.push_back(c);
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      out.push_back(c);
      break;
    }
  }
  return out;
}

} // namespace

bool ApiHandler::user_register_headers(const std::shared_ptr<Http2Connection> &conn,
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
  st.is_user_register = true;
  conn->streams[stream_id] = std::move(st);
  return true;
}

ApiHandler::ApiResponse ApiHandler::user_register_finish(std::span<const std::uint8_t> payload) {
  ApiResponse res;
  res.status = 400;
  res.content_type = "application/json";

  std::string body_str(payload.begin(), payload.end());
  std::string email = extract_json_string(body_str, "email");
  std::string display_name = extract_json_string(body_str, "display_name");

  if (email.empty()) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"email required\"}"sv.begin(),
                                         "{\"error\":\"email required\"}"sv.end());
    return res;
  }

  if (display_name.empty()) {
    display_name = email;
  }

  // Check user count limit (demo restriction)
  auto user_count = db_.count_users();
  if (user_count >= 1) {
    res.status = 403;
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"registration closed (demo limit: 1 user)\"}"sv.begin(),
        "{\"error\":\"registration closed (demo limit: 1 user)\"}"sv.end());
    return res;
  }

  // Check if user already exists
  auto existing = db_.find_user_by_email(email);
  if (existing) {
    res.body = std::vector<std::uint8_t>("{\"error\":\"user already exists\"}"sv.begin(),
                                         "{\"error\":\"user already exists\"}"sv.end());
    return res;
  }

  // Create pending user
  auto user = db_.create_user_pending(email, display_name);
  if (!user) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>("{\"error\":\"failed to create user\"}"sv.begin(),
                                         "{\"error\":\"failed to create user\"}"sv.end());
    return res;
  }

  // Generate verification token (32 bytes random)
  std::vector<std::uint8_t> token(32);
  if (RAND_bytes(token.data(), 32) != 1) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>("{\"error\":\"token generation failed\"}"sv.begin(),
                                         "{\"error\":\"token generation failed\"}"sv.end());
    return res;
  }

  // Hash token for storage
  std::vector<std::uint8_t> token_hash(SHA256_DIGEST_LENGTH);
  SHA256(token.data(), token.size(), token_hash.data());

  // Store verification with 24h TTL
  if (!db_.store_verification(token_hash, user->id, std::chrono::hours(24))) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>("{\"error\":\"failed to store verification\"}"sv.begin(),
                                         "{\"error\":\"failed to store verification\"}"sv.end());
    return res;
  }

  std::string token_b64 = base64url_encode(token);

  // Log for admin
  std::cerr << "\n[VERIFICATION] User: " << email << "\n"
            << "[VERIFICATION] Token: " << token_b64 << "\n"
            << "[VERIFICATION] Verify with: curl -X POST -d '{\"token\":\"" << token_b64
            << "\",\"email\":\"" << email << "\"}' https://$HOST/api/auth/user/verify\n\n";

  res.status = 200;
  std::ostringstream out;
  out << "{\"ok\":true,\"token\":\"" << token_b64 << "\"}";
  std::string resp = out.str();
  res.body = std::vector<std::uint8_t>(resp.begin(), resp.end());
  return res;
}

} // namespace neonsignal
