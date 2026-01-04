#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <iostream>

namespace neonsignal {

bool ApiHandler::events(const std::shared_ptr<Http2Connection>& conn,
                                   std::uint32_t stream_id,
                                   const std::string& path,
                                   const std::string& method,
                                   const std::string& authority) {
  conn->is_event_stream = true;
  conn->event_stream_id = stream_id;
  ++event_clients_;

  std::vector<std::uint8_t> headers_block;
  headers_block.push_back(0x88); // :status 200
  encode_literal_header_no_index(headers_block, 31, "text/event-stream");
  auto headers_frame = build_frame(0x1 /* HEADERS */, 0x4 /* END_HEADERS */,
                                   stream_id, headers_block);
  std::string body = "data: {\"files_served\": " +
                     std::to_string(served_files_.load()) +
                     ",\"page_views\": " +
                     std::to_string(page_views_.load()) + "}\n\n";
  std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
  auto data_frame = build_frame(0x0 /* DATA */, 0x0 /* no END_STREAM */,
                                stream_id, body_bytes);
  conn->write_buf.insert(conn->write_buf.end(), headers_frame.begin(),
                         headers_frame.end());
  conn->write_buf.insert(conn->write_buf.end(), data_frame.begin(),
                         data_frame.end());
  conn->events |= EPOLLOUT;
  loop_.update_fd(conn->fd, conn->events);
  std::cerr << "HEADERS on fd=" << conn->fd << " stream=" << stream_id
            << " path=" << path << " method=" << method
            << " authority=" << authority << " (sse)\n";
  return true;
}

} // namespace neonsignal
