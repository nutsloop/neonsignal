#include "install/install_command.h++"
#include "install/git_cloner.h++"

#include <filesystem>

namespace neonsignal::install {

bool InstallCommand::verify_clone_() const {
  // Determine the expected target path
  std::string target_name;
  if (target_name_ && !target_name_->empty()) {
    target_name = *target_name_;
  } else {
    target_name = GitCloner::extract_repo_name(repo_url_);
  }

  std::filesystem::path target_path = std::filesystem::path(www_root_) / target_name;

  // Check if the directory exists
  if (!std::filesystem::exists(target_path)) {
    return false;
  }

  // Check if it's a git repository (has .git directory)
  std::filesystem::path git_dir = target_path / ".git";
  return std::filesystem::exists(git_dir) && std::filesystem::is_directory(git_dir);
}

} // namespace neonsignal::install
