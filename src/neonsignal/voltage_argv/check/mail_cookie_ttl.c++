#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

unsigned long long check::mail_cookie_ttl() const {
  if (std::holds_alternative<unsigned long long>(this->value_)) {
    return std::get<unsigned long long>(this->value_);
  }

  if (std::holds_alternative<std::string>(this->value_)) {
    try {
      return std::stoull(std::get<std::string>(this->value_));
    } catch (const std::exception &) {
      throw std::invalid_argument(
          std::format("invalid value for --mail-cookie-ttl, expected integer seconds"));
    }
  }

  if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    const char *env_value = std::getenv("NEONSIGNAL_MAIL_COOKIE_TTL");
    if (env_value != nullptr) {
      try {
        return std::stoull(env_value);
      } catch (const std::exception &) {
        throw std::invalid_argument(
            std::format("NEONSIGNAL_MAIL_COOKIE_TTL expects integer seconds"));
      }
    }
    return 900;
  }

  throw std::invalid_argument(
      std::format("invalid argument type for --mail-cookie-ttl, expected integer"));
}

} // namespace neonsignal::voltage_argv
