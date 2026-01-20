#include "spin/api_handler.h++"

#include "spin/event_loop.h++"
#include "spin/event_mask.h++"
#include "spin/http2_listener_helpers.h++"

namespace neonsignal {

bool ApiHandler::codex_list(const std::shared_ptr<Http2Connection>& conn,
                            std::uint32_t stream_id) {
  auto items = db_.list_codex(25);
  std::string body = "{\"items\":[";
  for (std::size_t i = 0; i < items.size(); ++i) {
    const auto& item = items[i];
    body += "{";
    body += "\"id\":\"" + item.id + "\",";
    body += "\"title\":\"" + item.title + "\",";
    body += "\"created_at\":" + std::to_string(item.created_at) + ",";
    body += "\"bytes\":" + std::to_string(item.size) + ",";
    body += "\"has_image\":" + std::string(item.image_size ? "true" : "false");
    body += "}";
    if (i + 1 < items.size()) {
      body += ",";
    }
  }
  body += "]}";
  std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
  build_response_frames(conn->write_buf, stream_id, 200, "application/json", body_bytes);
  conn->events |= EventMask::Write;
  loop_.update_fd(conn->fd, conn->events);
  return true;
}

} // namespace neonsignal
