#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/event_mask.h++"
#include "neonsignal/http2_listener_helpers.h++"

namespace neonsignal {

bool ApiHandler::auth_login_options(
    const std::shared_ptr<Http2Connection>& conn, std::uint32_t stream_id) {
  auto opts = auth_.make_login_options();
  std::vector<std::uint8_t> body_bytes(opts.json.begin(), opts.json.end());
  build_response_frames(conn->write_buf, stream_id, 200, "application/json",
                        body_bytes);
  conn->events |= EventMask::Write;
  loop_.update_fd(conn->fd, conn->events);
  return true;
}

} // namespace neonsignal
