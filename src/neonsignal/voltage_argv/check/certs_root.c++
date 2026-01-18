#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::certs_root() const {
  std::string certs_root_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    certs_root_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_certs_root = std::getenv("NEONSIGNAL_CERTS_ROOT");
    if (env_certs_root != nullptr) {
      certs_root_val = env_certs_root;
    } else {
      certs_root_val = "./certs"; // Default
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --certs-root, expected string"));
  }

  // Validate path exists
  std::filesystem::path path(certs_root_val);
  if (!certs_root_val.empty() && !std::filesystem::exists(path)) {
    throw std::invalid_argument(
        std::format("--certs-root directory does not exist: '{}'", certs_root_val));
  }

  return certs_root_val;
}

} // namespace neonsignal::voltage_argv
