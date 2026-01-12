#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::webauthn_domain() const {
  std::string webauthn_domain_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    webauthn_domain_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_webauthn_domain = std::getenv("NEONSIGNAL_WEBAUTHN_DOMAIN");
    if (env_webauthn_domain != nullptr) {
      webauthn_domain_val = env_webauthn_domain;
    } else {
      webauthn_domain_val = ""; // Optional
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --webauthn-domain, expected string"));
  }

  return webauthn_domain_val;
}

} // namespace neonsignal::voltage_argv
