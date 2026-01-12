#include "neonsignal/codex_runner.h++"

#include <cctype>

namespace neonsignal {

bool CodexRunner::get_config_bool_(std::string_view key, bool fallback) const {
  auto value = db_.get_config(key);
  if (!value) {
    return fallback;
  }
  auto text = *value;
  for (auto &ch : text) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  if (text == "1" || text == "true" || text == "yes") {
    return true;
  }
  if (text == "0" || text == "false" || text == "no") {
    return false;
  }
  return fallback;
}

} // namespace neonsignal
