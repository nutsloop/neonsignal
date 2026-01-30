#include "neonsignal/voltage_argv/check.h++"

#include <cctype>
#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace neonsignal::voltage_argv {

namespace {

bool parse_bool(std::string_view value, std::string_view label) {
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
    value.remove_prefix(1);
  }
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
    value.remove_suffix(1);
  }

  std::string lower;
  lower.reserve(value.size());
  for (unsigned char c : value) {
    lower.push_back(static_cast<char>(std::tolower(c)));
  }
  if (lower == "1" || lower == "true" || lower == "yes" || lower == "on") {
    return true;
  }
  if (lower == "0" || lower == "false" || lower == "no" || lower == "off") {
    return false;
  }
  throw std::invalid_argument(std::format("{} expects a boolean value", label));
}

} // namespace

bool check::mail_enabled() const {
  if (std::holds_alternative<std::string>(this->value_)) {
    return parse_bool(std::get<std::string>(this->value_), "--mail-enabled");
  }
  if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    const char *env_value = std::getenv("NEONSIGNAL_MAIL_ENABLED");
    if (env_value != nullptr) {
      return parse_bool(env_value, "NEONSIGNAL_MAIL_ENABLED");
    }
    return false;
  }

  throw std::invalid_argument(
      std::format("invalid argument type for --mail-enabled, expected boolean"));
}

} // namespace neonsignal::voltage_argv
