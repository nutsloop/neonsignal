#include "spin/api_handler.h++"

#include "spin/event_loop.h++"
#include "spin/event_mask.h++"
#include "spin/http2_listener_helpers.h++"

#include <openssl/evp.h>

#include <array>
#include <cctype>
#include <string>

namespace neonsignal {

namespace {

struct CodexParsed {
  CodexBrief brief;
};

std::string trim_text(std::string_view value) {
  std::size_t start = 0;
  std::size_t end = value.size();
  while (start < end && std::isspace(static_cast<unsigned char>(value[start]))) {
    ++start;
  }
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }
  return std::string(value.substr(start, end - start));
}

std::string extract_param(std::string_view line, std::string_view key) {
  auto pos = line.find(key);
  if (pos == std::string_view::npos) {
    return {};
  }
  pos = line.find('"', pos);
  if (pos == std::string_view::npos) {
    return {};
  }
  auto end = line.find('"', pos + 1);
  if (end == std::string_view::npos) {
    return {};
  }
  return std::string(line.substr(pos + 1, end - pos - 1));
}

bool parse_multipart(std::span<const std::uint8_t> payload, std::string_view boundary,
                     CodexParsed& out) {
  if (boundary.empty()) {
    return false;
  }
  std::string marker = "--" + std::string(boundary);
  std::string_view body(reinterpret_cast<const char*>(payload.data()), payload.size());
  std::size_t pos = body.find(marker);
  while (pos != std::string_view::npos) {
    pos += marker.size();
    if (pos + 2 <= body.size() && body.substr(pos, 2) == "--") {
      break;
    }
    if (pos + 2 <= body.size() && body.substr(pos, 2) == "\r\n") {
      pos += 2;
    }
    std::size_t headers_end = body.find("\r\n\r\n", pos);
    if (headers_end == std::string_view::npos) {
      break;
    }
    std::string_view headers = body.substr(pos, headers_end - pos);
    std::size_t data_start = headers_end + 4;
    std::size_t next = body.find(marker, data_start);
    if (next == std::string_view::npos) {
      break;
    }
    std::size_t data_end = next;
    if (data_end >= 2 && body.substr(data_end - 2, 2) == "\r\n") {
      data_end -= 2;
    }
    std::string_view content = body.substr(data_start, data_end - data_start);

    std::string name;
    std::string filename;
    std::string content_type;
    std::size_t line_start = 0;
    while (line_start < headers.size()) {
      auto line_end = headers.find("\r\n", line_start);
      if (line_end == std::string_view::npos) {
        line_end = headers.size();
      }
      auto line = headers.substr(line_start, line_end - line_start);
      if (line.find("Content-Disposition:") != std::string_view::npos) {
        name = extract_param(line, "name=");
        filename = extract_param(line, "filename=");
      }
      if (line.find("Content-Type:") != std::string_view::npos) {
        auto pos_ct = line.find(':');
        if (pos_ct != std::string_view::npos) {
          content_type = trim_text(line.substr(pos_ct + 1));
        }
      }
      if (line_end == headers.size()) {
        break;
      }
      line_start = line_end + 2;
    }

    if (!filename.empty()) {
      out.brief.image_name = filename;
      out.brief.image_type = content_type;
      out.brief.image_bytes.assign(content.begin(), content.end());
    } else if (!name.empty()) {
      std::string value(content.begin(), content.end());
      if (name == "title") {
        out.brief.title = trim_text(value);
      } else if (name == "meta") {
        out.brief.meta_tags = trim_text(value);
      } else if (name == "description") {
        out.brief.description = trim_text(value);
      } else if (name == "fileRefs") {
        out.brief.file_refs = trim_text(value);
      } else if (name == "imageAlt") {
        out.brief.image_alt = trim_text(value);
      } else if (name == "imageMeta") {
        out.brief.image_meta = trim_text(value);
      }
    }
    pos = next;
  }
  return true;
}

std::string sha256_hex(std::span<const std::uint8_t> data) {
  std::array<std::uint8_t, 32> digest{};
  unsigned int len = 0;
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  if (!ctx) {
    return {};
  }
  if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
    EVP_MD_CTX_free(ctx);
    return {};
  }
  if (!data.empty()) {
    EVP_DigestUpdate(ctx, data.data(), data.size());
  }
  EVP_DigestFinal_ex(ctx, digest.data(), &len);
  EVP_MD_CTX_free(ctx);
  static const char* kHex = "0123456789abcdef";
  std::string out;
  out.reserve(len * 2);
  for (unsigned int i = 0; i < len; ++i) {
    out.push_back(kHex[(digest[i] >> 4) & 0xF]);
    out.push_back(kHex[digest[i] & 0xF]);
  }
  return out;
}

std::string extract_boundary(std::string_view content_type) {
  auto pos = content_type.find("boundary=");
  if (pos == std::string_view::npos) {
    return {};
  }
  auto boundary = content_type.substr(pos + 9);
  if (!boundary.empty() && boundary.front() == '"') {
    boundary.remove_prefix(1);
  }
  if (!boundary.empty() && boundary.back() == '"') {
    boundary.remove_suffix(1);
  }
  return std::string(boundary);
}

} // namespace

bool ApiHandler::codex_brief_headers(
    const std::shared_ptr<Http2Connection>& conn, std::uint32_t stream_id,
    const std::unordered_map<std::string, std::string>& headers,
    const std::string& path, const std::string& method) {
  if (method != "POST") {
    std::string body = "{\"error\":\"method not allowed\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 405, "application/json", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  Http2Connection::StreamState st{};
  st.path = path;
  st.method = method;
  st.expect_body = true;
  st.is_codex = true;
  auto ct = headers.find("content-type");
  if (ct != headers.end()) {
    st.content_type = ct->second;
  }
  conn->streams[stream_id] = std::move(st);
  return true;
}

ApiHandler::ApiResponse ApiHandler::codex_brief_finish(
    std::string_view content_type, std::span<const std::uint8_t> payload) {
  ApiResponse response;
  CodexParsed parsed{};
  if (content_type.find("multipart/form-data") != std::string_view::npos) {
    auto boundary = extract_boundary(content_type);
    parse_multipart(payload, boundary, parsed);
  }
  auto stored = db_.store_codex_entry(parsed.brief, content_type, payload);
  if (!stored) {
    response.status = 500;
    std::string body = "{\"error\":\"codex store failed\"}";
    response.body.assign(body.begin(), body.end());
    return response;
  }

  auto meta = db_.fetch_codex_record(stored->id);
  auto payload_check = db_.fetch_codex_payload(stored->id);
  bool verified = false;
  std::string verify_hash;
  if (meta && payload_check) {
    verify_hash = sha256_hex(*payload_check);
    verified = (meta->size == payload_check->size() && meta->sha256 == verify_hash);
  }

  std::string body = "{";
  body += "\"status\":\"queued\",";
  body += "\"codex_id\":\"" + stored->id + "\",";
  body += "\"content_type\":\"" + stored->content_type + "\",";
  body += "\"bytes\":" + std::to_string(stored->size) + ",";
  body += "\"sha256\":\"" + stored->sha256 + "\",";
  body += "\"verified\":" + std::string(verified ? "true" : "false") + ",";
  body += "\"message\":\"Codex queued. Response pending.\",";
  body += "\"title\":\"" + stored->title + "\",";
  body += "\"created_at\":" + std::to_string(stored->created_at);
  body += "}";
  response.body.assign(body.begin(), body.end());
  return response;
}

} // namespace neonsignal
