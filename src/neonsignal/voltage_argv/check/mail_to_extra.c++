#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::mail_to_extra() const {
  std::string value;

  if (std::holds_alternative<std::string>(this->value_)) {
    value = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    const char *env_value = std::getenv("NEONSIGNAL_MAIL_TO_EXTRA");
    if (env_value != nullptr) {
      value = env_value;
    } else {
      value = "";
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --mail-to-extra, expected string"));
  }

  return value;
}

} // namespace neonsignal::voltage_argv
