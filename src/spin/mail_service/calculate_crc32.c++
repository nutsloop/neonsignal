#include "spin/mail_service.h++"

#include <array>

namespace neonsignal {

std::uint32_t MailService::calculate_crc32(const std::string& data) {
  static const std::array<std::uint32_t, 256> table = []() {
    std::array<std::uint32_t, 256> t{};
    for (std::uint32_t i = 0; i < t.size(); ++i) {
      std::uint32_t c = i;
      for (int k = 0; k < 8; ++k) {
        if (c & 1u) {
          c = 0xEDB88320u ^ (c >> 1);
        } else {
          c >>= 1;
        }
      }
      t[i] = c;
    }
    return t;
  }();

  std::uint32_t crc = 0xFFFFFFFFu;
  for (unsigned char ch : data) {
    crc = (crc >> 8) ^ table[(crc ^ ch) & 0xFFu];
  }
  return crc ^ 0xFFFFFFFFu;
}

}  // namespace neonsignal
