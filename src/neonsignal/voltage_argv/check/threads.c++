#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <variant>

namespace neonsignal::voltage_argv {

unsigned long long check::threads() const {
  unsigned long long thread_count;

  if (std::holds_alternative<unsigned long long>(this->value_)) {
    thread_count = std::get<unsigned long long>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_threads = std::getenv("NEONSIGNAL_THREADS");
    if (env_threads != nullptr) {
      thread_count = std::stoull(env_threads);
    } else {
      thread_count = 4; // Default
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --threads, expected number"));
  }

  if (thread_count == 0) {
    throw std::invalid_argument("--threads must be greater than 0");
  }

  if (thread_count > 256) {
    throw std::out_of_range(
        std::format("--threads {} exceeds maximum allowed value of 256", thread_count));
  }

  return thread_count;
}

} // namespace neonsignal::voltage_argv
