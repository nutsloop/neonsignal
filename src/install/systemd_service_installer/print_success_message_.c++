#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <iostream>

namespace neonsignal::install {

void SystemdServiceInstaller::print_success_message_() const {
  using nutsloop::ansi;

  std::cerr << ansi("✓").bright_green().bold().str()
            << " systemd services installed\n\n";

  std::cerr << ansi("▸").bright_white().bold().str() << " Installed:\n";
  std::cerr << ansi("↳").bright_cyan().str()
            << " /etc/systemd/system/neonsignal.service\n";
  std::cerr << ansi("↳").bright_cyan().str()
            << " /etc/systemd/system/neonsignal_redirect.service\n\n";

  std::cerr << ansi("▸").bright_white().bold().str() << " Next steps:\n";
  std::cerr << ansi("↳").bright_yellow().str() << " systemctl daemon-reload\n";
  std::cerr << ansi("↳").bright_yellow().str()
            << " systemctl enable neonsignal neonsignal_redirect\n";
  std::cerr << ansi("↳").bright_yellow().str()
            << " systemctl start neonsignal neonsignal_redirect\n";
}

} // namespace neonsignal::install
