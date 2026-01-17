#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/event_mask.h++"
#include "neonsignal/http2_listener_helpers.h++"

namespace neonsignal {

bool ApiHandler::auth_user_check(
    const std::shared_ptr<Http2Connection>& conn, std::uint32_t stream_id,
    const std::unordered_map<std::string, std::string>& headers) {
  auto user_hdr = headers.find("x-user");
  std::string user = user_hdr == headers.end() ? "" : user_hdr->second;
  if (user.empty()) {
    std::string body = "{\"error\":\"missing user\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 400, "application/json",
                          body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  bool exists = auth_.user_exists(user);
  std::string body = std::string("{\"user\":\"") + user +
                     "\",\"exists\":" + (exists ? "true" : "false") + "}";
  std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
  build_response_frames(conn->write_buf, stream_id, 200, "application/json",
                        body_bytes);
  conn->events |= EventMask::Write;
  loop_.update_fd(conn->fd, conn->events);
  return true;
}

} // namespace neonsignal
