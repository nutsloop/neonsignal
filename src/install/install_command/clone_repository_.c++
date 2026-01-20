#include "install/install_command.h++"
#include "install/git_cloner.h++"

#include <filesystem>
#include <format>
#include <iostream>

namespace neonsignal::install {

bool InstallCommand::clone_repository_() {
  // Ensure www_root directory exists
  std::filesystem::path www_path(www_root_);
  if (!std::filesystem::exists(www_path)) {
    try {
      std::filesystem::create_directories(www_path);
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << std::format("Error: Failed to create www-root directory '{}': {}\n",
                               www_root_, e.what());
      return false;
    }
  }

  // Determine target directory name
  std::string target_name;
  if (target_name_ && !target_name_->empty()) {
    target_name = *target_name_;
  } else {
    target_name = GitCloner::extract_repo_name(repo_url_);
  }

  std::string target_path = (www_path / target_name).string();

  // Check if target already exists
  if (std::filesystem::exists(target_path)) {
    std::cerr << std::format("Error: Target directory already exists: '{}'\n", target_path);
    return false;
  }

  // Create cloner and execute
  GitCloner cloner(repo_url_, branch_);
  auto result = cloner.clone_to(target_path);

  if (!result) {
    return false;
  }

  std::cout << std::format("Cloned to: {}\n", *result);
  return true;
}

} // namespace neonsignal::install
