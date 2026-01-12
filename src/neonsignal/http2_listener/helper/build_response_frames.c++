#include "neonsignal/http2_listener_helpers.h++"

#include <string>
#include <vector>

namespace neonsignal {

void build_response_frames(std::vector<std::uint8_t> &out, std::uint32_t stream_id, int status,
                           std::string_view content_type, const std::vector<std::uint8_t> &body) {
  build_response_frames_with_headers(out, stream_id, status, content_type, {}, body);
}

void build_response_frames_with_headers(
    std::vector<std::uint8_t> &out, std::uint32_t stream_id, int status,
    std::string_view content_type,
    const std::vector<std::pair<std::string, std::string>> &extra_headers,
    const std::vector<std::uint8_t> &body) {
  std::vector<std::uint8_t> headers_block;

  // :status
  if (status == 200) {
    headers_block.push_back(0x88); // Indexed :status 200 (static table index 8)
  } else if (status == 404) {
    headers_block.push_back(0x8D); // Indexed :status 404 (static table index 13)
  } else if (status == 500) {
    headers_block.push_back(0x8E); // Indexed :status 500 (static table index 14)
  } else {
    // Literal :status with the given code using the :status name from static table.
    encode_literal_header_no_index(headers_block, 8, std::to_string(status));
  }

  // content-type
  encode_literal_header_no_index(headers_block, 31, content_type);
  auto encode_header = [&](const std::string &name, const std::string &value) {
    // Literal header without indexing, literal name (H=0).
    headers_block.push_back(0x00);
    encode_string(headers_block, name);
    encode_string(headers_block, value);
  };

  for (const auto &h : extra_headers) {
    encode_header(h.first, h.second);
  }

  auto headers_frame =
      build_frame(0x1 /* HEADERS */, 0x4 /* END_HEADERS */, stream_id, headers_block);
  out.insert(out.end(), headers_frame.begin(), headers_frame.end());

  // HTTP/2 default max frame size is 16384. Chunk DATA frames accordingly.
  constexpr std::size_t MAX_FRAME_SIZE = 16384;
  std::size_t offset = 0;
  while (offset < body.size()) {
    std::size_t chunk_size = std::min(MAX_FRAME_SIZE, body.size() - offset);
    bool is_last = (offset + chunk_size >= body.size());
    std::vector<std::uint8_t> chunk(body.begin() + offset, body.begin() + offset + chunk_size);
    std::uint8_t flags = is_last ? 0x1 /* END_STREAM */ : 0x0;
    auto data_frame = build_frame(0x0 /* DATA */, flags, stream_id, chunk);
    out.insert(out.end(), data_frame.begin(), data_frame.end());
    offset += chunk_size;
  }

  // Handle empty body case
  if (body.empty()) {
    auto data_frame = build_frame(0x0 /* DATA */, 0x1 /* END_STREAM */, stream_id, {});
    out.insert(out.end(), data_frame.begin(), data_frame.end());
  }
}

} // namespace neonsignal
