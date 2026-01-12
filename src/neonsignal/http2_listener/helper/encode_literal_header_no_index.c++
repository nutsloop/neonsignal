#include "neonsignal/http2_listener_helpers.h++"

#include <string_view>
#include <vector>

namespace neonsignal {

void encode_literal_header_no_index(std::vector<std::uint8_t> &out, std::uint32_t name_index,
                                    std::string_view value) {
  encode_integer(out, name_index, 4, 0x00);
  encode_string(out, value);
}

} // namespace neonsignal
