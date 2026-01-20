#include "install/install_command.h++"
#include "install/git_cloner.h++"
#include "neonsignal/logging.h++"

#include <filesystem>
#include <format>
#include <iostream>

namespace neonsignal::install {

int InstallCommand::run() {
  // Validate the repository URL
  if (!validate_repo_url_()) {
    std::cerr << "Error: Invalid repository URL\n";
    return 1;
  }

  // Clone the repository
  if (!clone_repository_()) {
    std::cerr << "Error: Failed to clone repository\n";
    return 1;
  }

  // Verify the clone was successful
  if (!verify_clone_()) {
    std::cerr << "Error: Clone verification failed\n";
    return 1;
  }

  std::cout << "Repository cloned successfully\n";
  return 0;
}

} // namespace neonsignal::install
