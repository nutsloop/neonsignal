#include "neonsignal/voltage_argv/check.h++"

#include <arpa/inet.h>
#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::redirect_host() const {
  std::string host_addr;

  if (std::holds_alternative<std::string>(this->value_)) {
    host_addr = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_host = std::getenv("REDIRECT_HOST");
    if (env_host != nullptr) {
      host_addr = env_host;
    } else {
      host_addr = "0.0.0.0"; // Default
    }
  } else {
    throw std::invalid_argument(std::format("invalid argument type for --host, expected string"));
  }

  // Validate IPv4 address
  in_addr_t addr{};
  if (inet_pton(AF_INET, host_addr.c_str(), &addr) <= 0) {
    throw std::invalid_argument(std::format("--host '{}' is not a valid IPv4 address", host_addr));
  }

  return host_addr;
}

} // namespace neonsignal::voltage_argv
