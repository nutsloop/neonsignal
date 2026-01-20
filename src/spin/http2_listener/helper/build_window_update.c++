#include "spin/http2_listener_helpers.h++"

#include <vector>

namespace neonsignal {

std::vector<std::uint8_t> build_window_update(std::uint32_t stream_id,
                                              std::uint32_t increment) {
  std::vector<std::uint8_t> payload;
  payload.push_back(static_cast<std::uint8_t>((increment >> 24) & 0x7F));
  payload.push_back(static_cast<std::uint8_t>((increment >> 16) & 0xFF));
  payload.push_back(static_cast<std::uint8_t>((increment >> 8) & 0xFF));
  payload.push_back(static_cast<std::uint8_t>(increment & 0xFF));
  return build_frame(0x8 /* WINDOW_UPDATE */, 0x0, stream_id, payload);
}

} // namespace neonsignal
