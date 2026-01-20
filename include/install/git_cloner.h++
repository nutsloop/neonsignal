#pragma once

#include <optional>
#include <string>

namespace neonsignal::install {

class GitCloner {
public:
  explicit GitCloner(std::string repo_url, std::optional<std::string> branch = std::nullopt);

  // Shallow clone (--depth=1) to target directory
  // Returns the path to the cloned directory on success, std::nullopt on failure
  [[nodiscard]] std::optional<std::string> clone_to(const std::string &target_dir);

  // Extract repository name from URL
  [[nodiscard]] static std::string extract_repo_name(const std::string &url);

private:
  std::string repo_url_;
  std::optional<std::string> branch_;

  [[nodiscard]] bool is_valid_url_() const;
  [[nodiscard]] bool execute_git_clone_(const std::string &target_path);
};

} // namespace neonsignal::install
