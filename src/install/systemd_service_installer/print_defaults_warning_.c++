#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <iostream>

namespace neonsignal::install {

void SystemdServiceInstaller::print_defaults_warning_() const {
  using nutsloop::ansi;

  std::cerr << ansi("▲").bright_yellow().bold().str()
            << " using default values for systemd service configuration\n\n";

  std::cerr << ansi("▸").bright_white().bold().str() << " "
            << ansi("neonsignal.service").bright_white().bold().str() << ":\n";
  std::cerr << ansi("↳").bright_cyan().str() << " User:             "
            << ansi(config_.user).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " Group:            "
            << ansi(config_.group).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " WorkingDirectory: "
            << ansi(config_.working_dir).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " Threads:          "
            << ansi(std::to_string(config_.threads)).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " Host:             "
            << ansi(config_.host).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " Port:             "
            << ansi(std::to_string(config_.port)).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " ExecPath:         "
            << ansi(config_.exec_path).bright_cyan().str() << "\n";

  std::cerr << "\n" << ansi("▸").bright_white().bold().str() << " "
            << ansi("neonsignal_redirect.service").bright_white().bold().str() << ":\n";
  std::cerr << ansi("↳").bright_cyan().str() << " Instances:        "
            << ansi(std::to_string(config_.redirect_instances)).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " Port:             "
            << ansi(std::to_string(config_.redirect_port)).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " TargetPort:       "
            << ansi(std::to_string(config_.redirect_target_port)).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " Host:             "
            << ansi(config_.redirect_host).bright_cyan().str() << "\n";
  std::cerr << ansi("↳").bright_cyan().str() << " ExecPath:         "
            << ansi(config_.redirect_exec_path).bright_cyan().str() << "\n";

  std::cerr << "\n"
            << ansi("▸").bright_cyan().str()
            << " customize with --systemd-service='user:myuser|group:mygroup|...'\n\n";
}

} // namespace neonsignal::install
