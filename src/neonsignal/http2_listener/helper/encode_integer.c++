#include "neonsignal/http2_listener_helpers.h++"

#include <vector>

namespace neonsignal {

void encode_integer(std::vector<std::uint8_t>& out, std::uint32_t value,
                    std::uint8_t prefix_bits, std::uint8_t first_byte_prefix) {
  const std::uint32_t max_prefix = (1u << prefix_bits) - 1u;
  if (value < max_prefix) {
    out.push_back(first_byte_prefix | static_cast<std::uint8_t>(value));
    return;
  }

  out.push_back(first_byte_prefix | static_cast<std::uint8_t>(max_prefix));
  value -= max_prefix;

  while (value >= 128) {
    out.push_back(static_cast<std::uint8_t>((value & 0x7F) | 0x80));
    value >>= 7;
  }
  out.push_back(static_cast<std::uint8_t>(value));
}

} // namespace neonsignal
