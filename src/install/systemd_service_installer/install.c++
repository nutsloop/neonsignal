#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <fstream>
#include <iostream>

namespace neonsignal::install {

int SystemdServiceInstaller::install() {
  using nutsloop::ansi;

#if !defined(__linux__)
  std::cerr << ansi("✗").bright_red().bold().str()
            << " systemd-service is only supported on Linux\n";
  return 1;
#endif

  // 1. Check root privileges
  if (!check_root_privileges_()) {
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

  // 5. Write to /etc/systemd/system/
  if (!write_service_file_("/etc/systemd/system/neonsignal.service", neonsignal_service)) {
    return 1;
  }
  if (!write_service_file_("/etc/systemd/system/neonsignal_redirect.service", redirect_service)) {
    return 1;
  }

  // 6. Print success
  print_success_message_();
  return 0;
}

} // namespace neonsignal::install
