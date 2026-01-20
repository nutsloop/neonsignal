#include "neonsignal/voltage_argv/check.h++"

#include <cctype>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::install_name() const {
  std::string name_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    name_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Name is optional, return empty string
    return "";
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --name, expected string"));
  }

  if (name_val.empty()) {
    return "";
  }

  // Validate directory name (no path separators, no special chars)
  for (char c : name_val) {
    if (c == '/' || c == '\\' || c == '\0') {
      throw std::invalid_argument(
          std::format("--name cannot contain path separators: '{}'", name_val));
    }
  }

  // Check for invalid names
  if (name_val == "." || name_val == "..") {
    throw std::invalid_argument(
        std::format("--name cannot be '.' or '..': '{}'", name_val));
  }

  return name_val;
}

} // namespace neonsignal::voltage_argv
