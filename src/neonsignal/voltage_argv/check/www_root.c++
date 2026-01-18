#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::www_root() const {
  std::string www_root_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    www_root_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_www_root = std::getenv("NEONSIGNAL_WWW_ROOT");
    if (env_www_root != nullptr) {
      www_root_val = env_www_root;
    } else {
      www_root_val = "./public"; // Default
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --www-root, expected string"));
  }

  // Validate path exists
  std::filesystem::path path(www_root_val);
  if (!www_root_val.empty() && !std::filesystem::exists(path)) {
    throw std::invalid_argument(
        std::format("--www-root directory does not exist: '{}'", www_root_val));
  }

  return www_root_val;
}

} // namespace neonsignal::voltage_argv
