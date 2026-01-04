#include "neonsignal/http2_listener_helpers.h++"

#include <vector>

namespace neonsignal {

std::vector<std::uint8_t> build_settings_ack() {
  std::vector<std::uint8_t> payload;
  return build_frame(0x4 /* SETTINGS */, 0x1 /* ACK */, 0, payload);
}

} // namespace neonsignal
