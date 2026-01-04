#include "neonsignal/http2_listener_helpers.h++"

#include <vector>

namespace neonsignal {

bool decode_integer(const std::vector<std::uint8_t>& buf, std::size_t& off,
                    std::uint8_t prefix_bits, std::uint32_t& out_val) {
  const std::uint32_t max_prefix = (1u << prefix_bits) - 1u;
  if (off >= buf.size()) {
    return false;
  }
  std::uint8_t byte = buf[off++];
  std::uint32_t val = byte & max_prefix;
  if (val != max_prefix) {
    out_val = val;
    return true;
  }

  std::uint32_t m = 0;
  do {
    if (off >= buf.size()) {
      return false;
    }
    byte = buf[off++];
    val += (byte & 0x7F) << m;
    m += 7;
  } while (byte & 0x80);

  out_val = val;
  return true;
}

} // namespace neonsignal
