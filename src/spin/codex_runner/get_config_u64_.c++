#include "spin/codex_runner.h++"

#include <string>

namespace neonsignal {

std::uint64_t CodexRunner::get_config_u64_(std::string_view key,
                                           std::uint64_t fallback) const {
  auto value = db_.get_config(key);
  if (!value || value->empty()) {
    return fallback;
  }
  try {
    return static_cast<std::uint64_t>(std::stoull(*value));
  } catch (...) {
    return fallback;
  }
}

} // namespace neonsignal
