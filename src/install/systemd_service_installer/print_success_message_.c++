#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <filesystem>
#include <iostream>

namespace neonsignal::install {

void SystemdServiceInstaller::print_success_message_() const {
  using nutsloop::ansi;

  if (only_save_path_) {
    const std::filesystem::path output_dir(*only_save_path_);

    std::cerr << ansi("✓").bright_green().bold().str()
              << " systemd service files saved\n\n";

    std::cerr << ansi("▸").bright_white().bold().str() << " Saved:\n";
    std::cerr << ansi("↳").bright_cyan().str()
              << " " << (output_dir / "neonsignal.service").string() << "\n";
    std::cerr << ansi("↳").bright_cyan().str()
              << " " << (output_dir / "neonsignal_redirect.service").string() << "\n\n";

    std::cerr << ansi("▸").bright_white().bold().str() << " Next steps:\n";
    std::cerr << ansi("↳").bright_yellow().str()
              << " sudo cp " << output_dir.string() << "/*.service /etc/systemd/system/\n";
    std::cerr << ansi("↳").bright_yellow().str() << " systemctl daemon-reload\n";
    std::cerr << ansi("↳").bright_yellow().str()
              << " systemctl enable neonsignal neonsignal_redirect\n";
    std::cerr << ansi("↳").bright_yellow().str()
              << " systemctl start neonsignal neonsignal_redirect\n";
    return;
  }

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
