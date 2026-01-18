#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

#include <unistd.h>

namespace neonsignal::voltage_argv {

std::string check::working_dir() const {
  std::string working_dir_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    working_dir_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    const char *env = std::getenv("NEONSIGNAL_WORKING_DIR");
    if (env != nullptr) {
      working_dir_val = env;
    }
    // No default - if not set, don't change working dir
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --working-dir, expected string"));
  }

  if (!working_dir_val.empty()) {
    std::filesystem::path path(working_dir_val);
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
      throw std::invalid_argument(
          std::format("--working-dir directory does not exist: '{}'", working_dir_val));
    }
    // Check write permission - server needs to create files in relative paths
    if (access(working_dir_val.c_str(), W_OK) != 0) {
      throw std::invalid_argument(
          std::format("--working-dir directory is not writable: '{}'", working_dir_val));
    }
  }

  return working_dir_val;
}

} // namespace neonsignal::voltage_argv
