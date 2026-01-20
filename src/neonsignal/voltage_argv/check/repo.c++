#include "neonsignal/voltage_argv/check.h++"

#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace neonsignal::voltage_argv {

std::string check::repo() const {
  std::string repo_val;

  if (std::holds_alternative<std::string>(this->value_)) {
    repo_val = std::get<std::string>(this->value_);
  } else if (std::holds_alternative<std::nullptr_t>(this->value_)) {
    throw std::invalid_argument("--repo is required for install command");
  } else {
    throw std::invalid_argument(
        std::format("invalid argument type for --repo, expected string"));
  }

  // Validate URL format (https:// or git@)
  if (repo_val.empty()) {
    throw std::invalid_argument("--repo cannot be empty");
  }

  bool valid_url = repo_val.starts_with("https://") ||
                   repo_val.starts_with("http://") ||
                   repo_val.starts_with("git@") ||
                   repo_val.starts_with("git://") ||
                   repo_val.starts_with("ssh://");

  if (!valid_url) {
    throw std::invalid_argument(
        std::format("--repo must be a valid git URL (https://, git@, etc.): '{}'", repo_val));
  }

  return repo_val;
}

} // namespace neonsignal::voltage_argv
