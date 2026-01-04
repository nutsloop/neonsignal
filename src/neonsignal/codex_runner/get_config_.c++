#include "neonsignal/codex_runner.h++"

namespace neonsignal {

std::string CodexRunner::get_config_(std::string_view key, std::string_view fallback) const {
  auto value = db_.get_config(key);
  if (!value || value->empty()) {
    return std::string(fallback);
  }
  return *value;
}

} // namespace neonsignal
