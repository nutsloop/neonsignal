#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <variant>

namespace neonsignal::voltage_argv {

unsigned long long check::instances() const {
  unsigned long long instance_count;

  if (std::holds_alternative<unsigned long long>(this->value_)) {
    instance_count = std::get<unsigned long long>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_instances = std::getenv("REDIRECT_INSTANCES");
    if (env_instances != nullptr) {
      instance_count = std::stoull(env_instances);
    } else {
      instance_count = 2; // Default
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --instances, expected number"));
  }

  if (instance_count == 0) {
    throw std::invalid_argument("--instances must be greater than 0");
  }

  if (instance_count > 256) {
    throw std::out_of_range(
        std::format("--instances {} exceeds maximum allowed value of 256", instance_count));
  }

  return instance_count;
}

} // namespace neonsignal::voltage_argv
