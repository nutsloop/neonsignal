#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <filesystem>
#include <iostream>

namespace neonsignal::install {

bool SystemdServiceInstaller::validate_config_() const {
  using nutsloop::ansi;

  // Check working directory exists
  if (!std::filesystem::exists(config_.working_dir)) {
    std::cerr << ansi("✗").bright_red().bold().str() << " working directory does not exist: "
              << ansi(config_.working_dir).bright_red().curly_underline().str() << "\n";
    return false;
  }

  if (!std::filesystem::is_directory(config_.working_dir)) {
    std::cerr << ansi("✗").bright_red().bold().str() << " working-dir is not a directory: "
              << ansi(config_.working_dir).bright_red().curly_underline().str() << "\n";
    return false;
  }

  // Check exec paths exist
  if (!std::filesystem::exists(config_.exec_path)) {
    std::cerr << ansi("✗").bright_red().bold().str() << " neonsignal executable not found: "
              << ansi(config_.exec_path).bright_red().curly_underline().str() << "\n";
    return false;
  }

  if (!std::filesystem::exists(config_.redirect_exec_path)) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " neonsignal_redirect executable not found: "
              << ansi(config_.redirect_exec_path).bright_red().curly_underline().str() << "\n";
    return false;
  }

  // Port validation
  if (config_.port == 0 || config_.port > 65535) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " invalid port number: " << config_.port << " (must be 1-65535)\n";
    return false;
  }

  if (config_.redirect_port == 0 || config_.redirect_port > 65535) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " invalid redirect-port number: " << config_.redirect_port << " (must be 1-65535)\n";
    return false;
  }

  if (config_.redirect_target_port == 0 || config_.redirect_target_port > 65535) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " invalid redirect-target-port number: " << config_.redirect_target_port
              << " (must be 1-65535)\n";
    return false;
  }

  // Threads validation
  if (config_.threads == 0 || config_.threads > 256) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " invalid threads number: " << config_.threads << " (must be 1-256)\n";
    return false;
  }

  // Redirect instances validation
  if (config_.redirect_instances == 0 || config_.redirect_instances > 256) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " invalid redirect-instances number: " << config_.redirect_instances
              << " (must be 1-256)\n";
    return false;
  }

  // User/group validation (basic check - not empty)
  if (config_.user.empty()) {
    std::cerr << ansi("✗").bright_red().bold().str() << " user cannot be empty\n";
    return false;
  }

  if (config_.group.empty()) {
    std::cerr << ansi("✗").bright_red().bold().str() << " group cannot be empty\n";
    return false;
  }

  return true;
}

} // namespace neonsignal::install
