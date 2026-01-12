#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <variant>

namespace neonsignal::voltage_argv {

unsigned long long check::redirect_port() const {
  unsigned long long port_num;

  if (std::holds_alternative<unsigned long long>(this->value_)) {
    port_num = std::get<unsigned long long>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_port = std::getenv("REDIRECT_PORT");
    if (env_port != nullptr) {
      port_num = std::stoull(env_port);
    } else {
      port_num = 80; // Default HTTP port
    }
  } else {
    throw std::invalid_argument(std::format("invalid argument type for --port, expected number"));
  }

  if (port_num == 0 || port_num > 65535) {
    throw std::out_of_range(std::format("--port {} is out of valid range (1-65535)", port_num));
  }

  return port_num;
}

} // namespace neonsignal::voltage_argv
