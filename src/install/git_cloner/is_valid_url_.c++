#include "install/git_cloner.h++"

namespace neonsignal::install {

bool GitCloner::is_valid_url_() const {
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
