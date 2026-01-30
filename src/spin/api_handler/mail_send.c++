#include "spin/api_handler.h++"

#include "spin/event_loop.h++"
#include "spin/event_mask.h++"
#include "spin/http2_listener_helpers.h++"
#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <netinet/in.h>
#include <optional>
#include <sstream>
#include <string_view>
#include <sys/socket.h>

namespace neonsignal {

using namespace std::string_view_literals;

namespace {

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
  std::string out;
  out.reserve(64);
  for (std::size_t i = pos + 1; i < json.size(); ++i) {
    char c = json[i];
    if (c == '"') {
      return out;
    }
    if (c == '\\' && i + 1 < json.size()) {
      char esc = json[++i];
      switch (esc) {
        case 'n':
          out.push_back('\n');
          break;
        case 'r':
          out.push_back('\r');
          break;
        case 't':
          out.push_back('\t');
          break;
        case '"':
          out.push_back('"');
          break;
        case '\\':
          out.push_back('\\');
          break;
        case '/':
          out.push_back('/');
          break;
        case 'b':
          out.push_back('\b');
          break;
        case 'f':
          out.push_back('\f');
          break;
        default:
          out.push_back(esc);
          break;
      }
      continue;
    }
    out.push_back(c);
  }
  return {};
}

std::optional<std::string> extract_cookie(
    const std::unordered_map<std::string, std::string>& headers,
    std::string_view name) {
  auto it = headers.find("cookie");
  if (it == headers.end()) {
    return std::nullopt;
  }
  std::string_view cookie = it->second;
  std::string needle = std::string(name) + "=";
  std::size_t pos = cookie.find(needle);
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  pos += needle.size();
  std::size_t end = cookie.find(';', pos);
  if (end == std::string_view::npos) {
    end = cookie.size();
  }
  return std::string(cookie.substr(pos, end - pos));
}

std::string normalize_authority(std::string_view authority) {
  if (auto pos = authority.find(':'); pos != std::string_view::npos) {
    authority = authority.substr(0, pos);
  }
  std::string out(authority);
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return out;
}

bool is_allowed_domain(std::string_view authority, const MailConfig& config) {
  std::string domain = normalize_authority(authority);
  std::string raw(authority);
  std::transform(raw.begin(), raw.end(), raw.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (config.allowed_domains.empty()) {
    return false;
  }
  for (const auto& allowed_value : config.allowed_domains) {
    std::string allowed = allowed_value;
    std::transform(allowed.begin(), allowed.end(), allowed.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (raw == allowed || domain == allowed) {
      return true;
    }
  }
  return false;
}

std::string get_client_ip(int fd) {
  sockaddr_storage addr{};
  socklen_t len = sizeof(addr);
  if (getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
    return {};
  }

  char buf[INET6_ADDRSTRLEN] = {};
  if (addr.ss_family == AF_INET) {
    const auto* in = reinterpret_cast<const sockaddr_in*>(&addr);
    if (!inet_ntop(AF_INET, &in->sin_addr, buf, sizeof(buf))) {
      return {};
    }
  } else if (addr.ss_family == AF_INET6) {
    const auto* in6 = reinterpret_cast<const sockaddr_in6*>(&addr);
    if (!inet_ntop(AF_INET6, &in6->sin6_addr, buf, sizeof(buf))) {
      return {};
    }
  } else {
    return {};
  }
  return std::string(buf);
}

bool is_valid_email(std::string_view email) {
  if (email.empty() || email.size() > 320) {
    return false;
  }
  if (email.find(' ') != std::string_view::npos ||
      email.find('\n') != std::string_view::npos ||
      email.find('\r') != std::string_view::npos) {
    return false;
  }
  auto at = email.find('@');
  if (at == std::string_view::npos || at == 0 || at + 1 >= email.size()) {
    return false;
  }
  auto dot = email.find('.', at + 1);
  if (dot == std::string_view::npos || dot + 1 >= email.size()) {
    return false;
  }
  return true;
}

bool has_crlf(std::string_view value) {
  return value.find('\n') != std::string_view::npos ||
         value.find('\r') != std::string_view::npos;
}

std::string normalize_form(std::string form) {
  if (!form.empty() && form.front() == '/') {
    form.erase(form.begin());
  }
  return form;
}

bool is_allowed_form(std::string_view form, const MailConfig& config) {
  if (config.url_hits.empty()) {
    return false;
  }
  for (const auto& hit : config.url_hits) {
    std::string_view hit_view(hit);
    if (!hit_view.empty() && hit_view.front() == '/') {
      hit_view.remove_prefix(1);
    }
    if (form == hit_view) {
      return true;
    }
  }
  return false;
}

std::string normalize_message(std::string message) {
  std::string out;
  out.reserve(message.size());
  for (std::size_t i = 0; i < message.size(); ++i) {
    if (message[i] == '\\' && i + 1 < message.size()) {
      char next = message[i + 1];
      if (next == 'n') {
        out.push_back('\n');
        ++i;
        continue;
      }
      if (next == 'r') {
        out.push_back('\r');
        ++i;
        continue;
      }
      if (next == 't') {
        out.push_back('\t');
        ++i;
        continue;
      }
    }
    out.push_back(message[i]);
  }
  return out;
}

} // namespace

bool ApiHandler::mail_send_headers(const std::shared_ptr<Http2Connection>& conn,
                                   std::uint32_t stream_id,
                                   const std::unordered_map<std::string, std::string>& headers,
                                   const std::string& path,
                                   const std::string& method,
                                   const std::string& authority) {
  if (!mail_config_.enabled) {
    std::string body = "{\"error\":\"mail disabled\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 404, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  if (method != "POST") {
    std::string body = "{\"error\":\"method not allowed\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 405, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  if (!is_allowed_domain(authority, mail_config_)) {
    std::string body = "{\"error\":\"requests from this domain are not allowed\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 403, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  std::string client_ip = get_client_ip(conn->fd);
  if (client_ip.empty()) {
    std::string body = "{\"error\":\"missing or invalid anti-spam cookie\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 403, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  if (!mail_config_.allowed_ip_address.empty() &&
      client_ip != mail_config_.allowed_ip_address) {
    std::string body = "{\"error\":\"requests from this ip are not allowed\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 403, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  auto cookie_code = extract_cookie(headers, mail_config_.cookie_name);
  if (!cookie_code || cookie_code->empty() ||
      !mail_cookie_store_.validate(client_ip, *cookie_code)) {
    std::string body = "{\"error\":\"missing or invalid anti-spam cookie\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 403, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  Http2Connection::StreamState st{};
  st.path = path;
  st.method = method;
  st.expect_body = true;
  st.is_mail_send = true;
  st.mail_cookie_code = *cookie_code;
  st.client_ip = std::move(client_ip);
  conn->streams[stream_id] = std::move(st);
  return true;
}

ApiHandler::ApiResponse ApiHandler::mail_send_finish(const std::string& client_ip,
                                                     std::span<const std::uint8_t> payload,
                                                     const std::string& cookie_code) {
  ApiResponse res;
  res.status = 400;
  res.content_type = "application/json";

  std::string body_str(payload.begin(), payload.end());
  std::string name = extract_json_string(body_str, "name");
  std::string email = extract_json_string(body_str, "email");
  std::string subject = extract_json_string(body_str, "subject");
  std::string message = normalize_message(extract_json_string(body_str, "message"));
  std::string form = normalize_form(extract_json_string(body_str, "form"));

  if (client_ip.empty() || cookie_code.empty()) {
    res.status = 403;
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"missing or invalid anti-spam cookie\"}"sv.begin(),
        "{\"error\":\"missing or invalid anti-spam cookie\"}"sv.end());
    return res;
  }

  if (!is_valid_email(email)) {
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"invalid email address\"}"sv.begin(),
        "{\"error\":\"invalid email address\"}"sv.end());
    return res;
  }

  if (message.empty()) {
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"message required\"}"sv.begin(),
        "{\"error\":\"message required\"}"sv.end());
    return res;
  }

  if (!is_allowed_form(form, mail_config_)) {
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"invalid form\"}"sv.begin(),
        "{\"error\":\"invalid form\"}"sv.end());
    return res;
  }

  constexpr std::size_t kMaxName = 120;
  constexpr std::size_t kMaxSubject = 200;
  constexpr std::size_t kMaxMessage = 8000;

  if (name.size() > kMaxName || subject.size() > kMaxSubject || message.size() > kMaxMessage) {
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"message too large\"}"sv.begin(),
        "{\"error\":\"message too large\"}"sv.end());
    return res;
  }

  if (has_crlf(email) || has_crlf(subject) || has_crlf(name)) {
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"invalid input\"}"sv.begin(),
        "{\"error\":\"invalid input\"}"sv.end());
    return res;
  }

  if (subject.empty()) {
    subject = (form == "enroll.html") ? "Enrollment request" : "Contact request";
  }

  std::ostringstream body;
  body << "Form: " << form << "\n";
  if (!name.empty()) {
    body << "Name: " << name << "\n";
  }
  body << "Email: " << email << "\n";
  body << "IP: " << client_ip << "\n\n";
  body << message;

  MailService::MailRequest request;
  request.client_ip = client_ip;
  request.cookie_code = cookie_code;
  request.form = form;
  request.from = email;
  request.name = name;
  request.subject = subject;
  request.message = body.str();

  auto mail_id = mail_service_.send_async(request);
  if (!mail_id) {
    res.status = 500;
    res.body = std::vector<std::uint8_t>(
        "{\"error\":\"failed to send mail\"}"sv.begin(),
        "{\"error\":\"failed to send mail\"}"sv.end());
    return res;
  }

  res.status = 200;
  std::string resp = "{\"ok\":true,\"mail_id\":\"" + *mail_id + "\"}";
  res.body = std::vector<std::uint8_t>(resp.begin(), resp.end());
  return res;
}

} // namespace neonsignal
