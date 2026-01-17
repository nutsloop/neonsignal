#include "neonsignal/voltage_argv/check.h++"

#include <format>
#include <stdexcept>
#include <variant>

namespace neonsignal::voltage_argv {

bool check::systemd() const {
  bool value;

  if (std::holds_alternative<bool>(this->value_)) {
    value = std::get<bool>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    value = false;
  } else {
    throw std::invalid_argument(
        std::format("--systemd is a simple switch that should omit the `=` sign"));
  }

  return value;
}

} // namespace neonsignal::voltage_argv
