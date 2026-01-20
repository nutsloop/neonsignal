#include "spin/api_handler.h++"

#include "spin/event_loop.h++"
#include "spin/event_mask.h++"
#include "spin/http2_listener_helpers.h++"

#include <string_view>

namespace neonsignal {

namespace {

std::string query_param(std::string_view path, std::string_view key) {
  auto qpos = path.find('?');
  if (qpos == std::string_view::npos) {
    return {};
  }
  std::string_view query = path.substr(qpos + 1);
  while (!query.empty()) {
    auto amp = query.find('&');
    std::string_view chunk = query.substr(0, amp);
    auto eq = chunk.find('=');
    if (eq != std::string_view::npos) {
      auto k = chunk.substr(0, eq);
      auto v = chunk.substr(eq + 1);
      if (k == key) {
        return std::string(v);
      }
    }
    if (amp == std::string_view::npos) {
      break;
    }
    query.remove_prefix(amp + 1);
  }
  return {};
}

} // namespace

bool ApiHandler::codex_image(const std::shared_ptr<Http2Connection>& conn,
                             std::uint32_t stream_id, const std::string& path) {
  auto id = query_param(path, "id");
  if (id.empty()) {
    std::string body = "missing id";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 400, "text/plain; charset=utf-8",
                          body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }
  auto record = db_.fetch_codex_record(id);
  if (!record || record->image_size == 0) {
    std::string body = "not found";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 404, "text/plain; charset=utf-8",
                          body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }
  auto bytes = db_.fetch_codex_image(id);
  if (!bytes) {
    std::string body = "not found";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 404, "text/plain; charset=utf-8",
                          body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }
  std::string content_type = record->image_type.empty()
                                 ? "application/octet-stream"
                                 : record->image_type;
  build_response_frames(conn->write_buf, stream_id, 200, content_type, *bytes);
  conn->events |= EventMask::Write;
  loop_.update_fd(conn->fd, conn->events);
  return true;
}

} // namespace neonsignal
