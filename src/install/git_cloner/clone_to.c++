#include "install/git_cloner.h++"

#include <format>
#include <iostream>
#include <optional>
#include <string>

namespace neonsignal::install {

std::optional<std::string> GitCloner::clone_to(const std::string &target_dir) {
  // Validate URL before cloning
  if (!is_valid_url_()) {
    std::cerr << std::format("✗ Invalid repository URL: '{}'\n", repo_url_);
    return std::nullopt;
  }

  // Execute the git clone command
  if (!execute_git_clone_(target_dir)) {
    return std::nullopt;
  }

  return target_dir;
}

} // namespace neonsignal::install
