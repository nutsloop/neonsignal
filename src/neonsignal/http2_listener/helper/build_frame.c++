#include "neonsignal/http2_listener_helpers.h++"

#include <algorithm>
#include <vector>

namespace neonsignal {

std::vector<std::uint8_t> build_frame(std::uint8_t type, std::uint8_t flags,
                                      std::uint32_t stream_id,
                                      const std::vector<std::uint8_t> &payload) {
  const auto len = static_cast<std::uint32_t>(payload.size());
  std::vector<std::uint8_t> frame;
  frame.reserve(9 + len);

  // Header buffer
  std::uint8_t header[9];

  // Length field: 24 bits, network byte order.
  header[0] = static_cast<std::uint8_t>((len >> 16) & 0xFF);
  header[1] = static_cast<std::uint8_t>((len >> 8) & 0xFF);
  header[2] = static_cast<std::uint8_t>(len & 0xFF);

  // Type and flags.
  header[3] = type;
  header[4] = flags;

  // Stream ID: 31 bits (highest bit must be 0).
  header[5] = static_cast<std::uint8_t>((stream_id >> 24) & 0x7F);
  header[6] = static_cast<std::uint8_t>((stream_id >> 16) & 0xFF);
  header[7] = static_cast<std::uint8_t>((stream_id >> 8) & 0xFF);
  header[8] = static_cast<std::uint8_t>(stream_id & 0xFF);

  // Insert header and payload
  frame.insert(frame.end(), header, header + 9);
  frame.insert(frame.end(), payload.begin(), payload.end());

  return frame;
}

} // namespace neonsignal
