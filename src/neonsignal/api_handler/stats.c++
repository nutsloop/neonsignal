#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/event_mask.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <iostream>

namespace neonsignal {

bool ApiHandler::stats(const std::shared_ptr<Http2Connection>& conn,
                                  std::uint32_t stream_id,
                                  const std::string& path,
                                  const std::string& method,
                                  const std::string& authority) {
  std::string body = "{\"files_served\":" +
                     std::to_string(served_files_.load()) +
                     ",\"page_views\":" +
                     std::to_string(page_views_.load()) + "}";
  std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
  build_response_frames(conn->write_buf, stream_id, 200, "application/json",
                        body_bytes);
  conn->events |= EventMask::Write;
  loop_.update_fd(conn->fd, conn->events);
  std::cerr << "HEADERS on fd=" << conn->fd << " stream=" << stream_id
            << " path=" << path << " method=" << method
            << " authority=" << authority << " (api)\n";
  return true;
}

} // namespace neonsignal
