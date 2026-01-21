#include "install/systemd_service_installer.h++"

#include <ansi.h++>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace neonsignal::install {

bool SystemdServiceInstaller::write_service_file_(const std::string &path,
                                                   const std::string &content) const {
  using nutsloop::ansi;

  std::filesystem::path file_path(path);
  if (!file_path.parent_path().empty()) {
    std::error_code ec;
    std::filesystem::create_directories(file_path.parent_path(), ec);
    if (ec) {
      std::cerr << ansi("✗").bright_red().bold().str()
                << " failed to create directory: "
                << ansi(file_path.parent_path().string()).bright_red().curly_underline().str()
                << "\n";
      return false;
    }
  }

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
