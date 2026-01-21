#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace neonsignal::install {

int SystemdServiceInstaller::install() {
  using nutsloop::ansi;

#if !defined(__linux__)
  if (!only_save_path_) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " systemd-service installs are only supported on Linux\n";
    std::cerr << ansi("▸").bright_cyan().str()
              << " use --only-save to generate service files on this platform\n";
    return 1;
  }
#endif

  // 1. Check root privileges (only required when installing to system paths)
  if (!only_save_path_ && !check_root_privileges_()) {
    std::cerr << ansi("✗").bright_red().bold().str()
              << " must run as root to install systemd services\n";
    std::cerr << ansi("▸").bright_cyan().str()
              << " use sudo neonsignal install --systemd-service\n";
    return 1;
  }

  // 2. Warn about defaults if applicable
  if (using_defaults_) {
    print_defaults_warning_();
  }

  // 3. Validate configuration
  if (!validate_config_()) {
    return 1;
  }

  // 4. Generate service files
  std::string neonsignal_service = generate_neonsignal_service_();
  std::string redirect_service = generate_redirect_service_();

  // 5. Write service files
  const std::filesystem::path output_dir =
      only_save_path_ ? std::filesystem::path(*only_save_path_) : "/etc/systemd/system";
  const std::filesystem::path neonsignal_path = output_dir / "neonsignal.service";
  const std::filesystem::path redirect_path = output_dir / "neonsignal_redirect.service";

  if (!write_service_file_(neonsignal_path.string(), neonsignal_service)) {
    return 1;
  }
  if (!write_service_file_(redirect_path.string(), redirect_service)) {
    return 1;
  }

  // 6. Print success
  print_success_message_();
  return 0;
}

} // namespace neonsignal::install
