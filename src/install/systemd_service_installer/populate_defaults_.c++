#include "install/systemd_service_installer.h++"

#include <filesystem>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

namespace neonsignal::install {

void SystemdServiceInstaller::populate_defaults_() {
  // Current user
  if (struct passwd *pw = getpwuid(getuid()); pw != nullptr) {
    config_.user = pw->pw_name;
  } else {
    config_.user = "root";
  }

  // Current group
  if (struct group *gr = getgrgid(getgid()); gr != nullptr) {
    config_.group = gr->gr_name;
  } else {
    config_.group = "root";
  }

  // Current working directory
  config_.working_dir = std::filesystem::current_path().string();

  // redirect_host defaults to host
  config_.redirect_host = config_.host;
}

} // namespace neonsignal::install
