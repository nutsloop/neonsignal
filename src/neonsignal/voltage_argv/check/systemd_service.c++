#include "neonsignal/voltage_argv/check.h++"

#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::systemd_service() const {
  // systemd-service can be used as a flag (no value) or with KVP string
  if (std::holds_alternative<std::string>(this->value_)) {
    return std::get<std::string>(this->value_);
  }

  if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Flag used without value - return empty string to indicate defaults
    return "";
  }

  if (std::holds_alternative<bool>(this->value_)) {
    // Flag used as boolean - return empty string to indicate defaults
    return "";
  }

  // Unexpected type
  return "";
}

} // namespace neonsignal::voltage_argv
