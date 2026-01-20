#include "spin/http2_listener_helpers.h++"

#include <string_view>
#include <vector>

namespace neonsignal {

void encode_string(std::vector<std::uint8_t>& out, std::string_view str) {
  // No Huffman encoding; H=0.
  encode_integer(out, static_cast<std::uint32_t>(str.size()), 7, 0x00);
  out.insert(out.end(), str.begin(), str.end());
}

} // namespace neonsignal
