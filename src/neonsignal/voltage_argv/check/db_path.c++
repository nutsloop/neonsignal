#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::db_path() const {
  std::string db_path_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    db_path_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_db_path = std::getenv("NEONSIGNAL_DB_PATH");
    if (env_db_path != nullptr) {
      db_path_val = env_db_path;
    } else {
      db_path_val = "./data"; // Default
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --db-path, expected string"));
  }

  // Validate path exists or can be created
  std::filesystem::path path(db_path_val);
  if (!db_path_val.empty()) {
    // Check if parent directory exists
    auto parent = path.parent_path();
    if (!parent.empty() && !std::filesystem::exists(parent)) {
      throw std::invalid_argument(
          std::format("--db-path parent directory does not exist: '{}'", parent.string()));
    }
  }

  return db_path_val;
}

} // namespace neonsignal::voltage_argv
