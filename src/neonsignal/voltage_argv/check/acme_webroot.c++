#include "neonsignal/voltage_argv/check.h++"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::acme_webroot() const {
  std::string acme_root;

  if (std::holds_alternative<std::string>(this->value_)) {
    acme_root = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Check environment variable
    const char *env_acme = std::getenv("ACME_WEBROOT");
    if (env_acme != nullptr) {
      acme_root = env_acme;
    } else {
      acme_root = ""; // Optional
    }
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --acme-webroot, expected string"));
  }

  // Validate path exists (if provided)
  if (!acme_root.empty()) {
    std::filesystem::path path(acme_root);
    if (!std::filesystem::exists(path)) {
      throw std::invalid_argument(
          std::format("--acme-webroot directory does not exist: '{}'", acme_root));
    }
    if (!std::filesystem::is_directory(path)) {
      throw std::invalid_argument(
          std::format("--acme-webroot is not a directory: '{}'", acme_root));
    }
  }

  return acme_root;
}

} // namespace neonsignal::voltage_argv
