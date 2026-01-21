#include "neonsignal/voltage_argv/check.h++"

#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::only_save_path() const {
  if (std::holds_alternative<std::string>(this->value_)) {
    return std::get<std::string>(this->value_);
  }

  if (std::holds_alternative<std::nullptr_t>(this->value_) ||
      std::holds_alternative<bool>(this->value_)) {
    return "";
  }

  return "";
}

} // namespace neonsignal::voltage_argv
