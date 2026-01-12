#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::webauthn_origin() const {
  std::string webauthn_origin_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    webauthn_origin_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_webauthn_origin = std::getenv("NEONSIGNAL_WEBAUTHN_ORIGIN");
    if (env_webauthn_origin != nullptr) {
      webauthn_origin_val = env_webauthn_origin;
    } else {
      webauthn_origin_val = ""; // Optional
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --webauthn-origin, expected string"));
  }

  // Basic URL validation if provided
  if (!webauthn_origin_val.empty() && webauthn_origin_val.substr(0, 8) != "https://" &&
      webauthn_origin_val.substr(0, 7) != "http://") {
    throw std::invalid_argument(
        std::format("--webauthn-origin '{}' must be a valid URL starting with http:// or https://",
                    webauthn_origin_val));
  }

  return webauthn_origin_val;
}

} // namespace neonsignal::voltage_argv
