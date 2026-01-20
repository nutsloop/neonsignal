#include "spin/api_handler.h++"

#include "spin/event_loop.h++"
#include "spin/event_mask.h++"
#include "spin/http2_listener_helpers.h++"

#include <iostream>

namespace neonsignal {

bool ApiHandler::redirect_service_stream(
    const std::shared_ptr<Http2Connection>& conn, std::uint32_t stream_id,
    const std::string& path, const std::string& method,
    const std::string& authority) {
  if (method != "GET") {
    std::string body = "Method Not Allowed";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 405,
                          "text/plain; charset=utf-8", body_bytes);
    conn->events |= EventMask::Write;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }
  conn->is_redirect_stream = true;
  conn->redirect_stream_id = stream_id;

  std::vector<std::uint8_t> headers_block;
  headers_block.push_back(0x88); // :status 200
  encode_literal_header_no_index(headers_block, 31, "text/event-stream");
  auto headers_frame = build_frame(0x1 /* HEADERS */, 0x4 /* END_HEADERS */,
                                   stream_id, headers_block);
  bool ok = redirect_service_ok_.load();
  std::string body = "data: {\"redirect_ok\": " +
                     std::string(ok ? "true" : "false") + "}\n\n";
  std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
  auto data_frame = build_frame(0x0 /* DATA */, 0x0 /* no END_STREAM */,
                                stream_id, body_bytes);
  conn->write_buf.insert(conn->write_buf.end(), headers_frame.begin(),
                         headers_frame.end());
  conn->write_buf.insert(conn->write_buf.end(), data_frame.begin(),
                         data_frame.end());
  conn->events |= EventMask::Write;
  loop_.update_fd(conn->fd, conn->events);
  std::cerr << "HEADERS on fd=" << conn->fd << " stream=" << stream_id
            << " path=" << path << " method=" << method
            << " authority=" << authority << " (redirect sse)\n";
  return true;
}

} // namespace neonsignal
