#include "neonsignal/voltage_argv/check.h++"

#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::branch() const {
  std::string branch_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    branch_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    // Branch is optional, return empty string (will use default branch)
    return "";
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --branch, expected string"));
  }

  if (branch_val.empty()) {
    return "";
  }

  // Validate branch name (basic validation)
  // Git branch names cannot start with '-', contain '..', or end with '/'
  if (branch_val.starts_with("-")) {
    throw std::invalid_argument(
        std::format("--branch cannot start with '-': '{}'", branch_val));
  }

  if (branch_val.find("..") != std::string::npos) {
    throw std::invalid_argument(
        std::format("--branch cannot contain '..': '{}'", branch_val));
  }

  if (branch_val.ends_with("/")) {
    throw std::invalid_argument(
        std::format("--branch cannot end with '/': '{}'", branch_val));
  }

  return branch_val;
}

} // namespace neonsignal::voltage_argv
