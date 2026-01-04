#include "neonsignal/http2_listener_helpers.h++"

#include <vector>

namespace neonsignal {

std::vector<std::uint8_t> build_server_settings() {
  std::vector<std::uint8_t> payload;
  // SETTINGS_MAX_CONCURRENT_STREAMS = 100
  payload.push_back(0x00);
  payload.push_back(0x03);
  payload.push_back(0x00);
  payload.push_back(0x00);
  payload.push_back(0x00);
  payload.push_back(0x64);
  // SETTINGS_INITIAL_WINDOW_SIZE = 16MB (0x01000000)
  payload.push_back(0x00);
  payload.push_back(0x04);
  payload.push_back(0x01);
  payload.push_back(0x00);
  payload.push_back(0x00);
  payload.push_back(0x00);
  return build_frame(0x4 /* SETTINGS */, 0x0, 0, payload);
}

} // namespace neonsignal
