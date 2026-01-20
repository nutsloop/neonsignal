#include "install/install_command.h++"

#include <string>

namespace neonsignal::install {

bool InstallCommand::validate_repo_url_() const {
  if (repo_url_.empty()) {
    return false;
  }

  // Check for valid URL schemes
  return repo_url_.starts_with("https://") ||
         repo_url_.starts_with("http://") ||
         repo_url_.starts_with("git@") ||
         repo_url_.starts_with("git://") ||
         repo_url_.starts_with("ssh://");
}

} // namespace neonsignal::install
