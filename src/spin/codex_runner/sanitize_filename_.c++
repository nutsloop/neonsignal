#include "spin/codex_runner.h++"

#include <cctype>

namespace neonsignal {

std::string CodexRunner::sanitize_filename_(std::string_view name) const {
  std::string out;
  out.reserve(name.size());
  for (char ch : name) {
    if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '.' || ch == '_' || ch == '-') {
      out.push_back(ch);
    } else if (ch == ' ') {
      out.push_back('_');
    }
  }
  if (out.empty()) {
    out = "image.bin";
  }
  return out;
}

} // namespace neonsignal
