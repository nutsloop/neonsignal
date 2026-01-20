#include "install/git_cloner.h++"

#include <array>
#include <cstdio>
#include <format>
#include <iostream>
#include <memory>
#include <string>

namespace neonsignal::install {

bool GitCloner::execute_git_clone_(const std::string &target_path) {
  // Build the git clone command
  // Use --depth=1 for shallow clone
  std::string command = "git clone --depth=1";

  // Add branch if specified
  if (branch_ && !branch_->empty()) {
    command += std::format(" --branch={}", *branch_);
  }

  // Add repository URL and target path
  command += std::format(" \"{}\" \"{}\" 2>&1", repo_url_, target_path);

  std::cout << std::format("Executing: {}\n", command);

  // Execute the command and capture output
  std::array<char, 256> buffer;
  std::string output;

  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
  if (!pipe) {
    std::cerr << "✗ Failed to execute git clone command\n";
    return false;
  }

  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
    output += buffer.data();
    std::cout << buffer.data();
  }

  int exit_code = pclose(pipe.release());
  if (exit_code != 0) {
    std::cerr << std::format("✗ git clone failed with exit code {}\n", exit_code);
    return false;
  }

  return true;
}

} // namespace neonsignal::install
