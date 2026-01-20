#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <fstream>
#include <iostream>

namespace neonsignal::install {

bool SystemdServiceInstaller::write_service_file_(const std::string &path,
                                                   const std::string &content) const {
  using nutsloop::ansi;

  std::ofstream file(path);
  if (!file) {
    std::cerr << ansi("✗").bright_red().bold().str() << " failed to open file for writing: "
              << ansi(path).bright_red().curly_underline().str() << "\n";
    return false;
  }

  file << content;

  if (!file.good()) {
    std::cerr << ansi("✗").bright_red().bold().str() << " failed to write to file: "
              << ansi(path).bright_red().curly_underline().str() << "\n";
    return false;
  }

  file.close();
  return true;
}

} // namespace neonsignal::install
