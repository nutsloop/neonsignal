#include "install/git_cloner.h++"

#include <string>

namespace neonsignal::install {

std::string GitCloner::extract_repo_name(const std::string &url) {
  // Handle empty URL
  if (url.empty()) {
    return "repo";
  }

  std::string name = url;

  // Remove trailing .git if present
  if (name.ends_with(".git")) {
    name = name.substr(0, name.size() - 4);
  }

  // Remove trailing slash if present
  if (name.ends_with("/")) {
    name = name.substr(0, name.size() - 1);
  }

  // Find the last path component
  std::size_t last_slash = name.rfind('/');
  std::size_t last_colon = name.rfind(':');

  // For git@github.com:user/repo format
  std::size_t start_pos = 0;
  if (last_slash != std::string::npos) {
    start_pos = last_slash + 1;
  } else if (last_colon != std::string::npos) {
    start_pos = last_colon + 1;
  }

  name = name.substr(start_pos);

  // Fallback to "repo" if name is empty
  if (name.empty()) {
    return "repo";
  }

  return name;
}

} // namespace neonsignal::install
