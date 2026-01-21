#include "install/systemd_service_installer.h++"

#include <stdexcept>

namespace neonsignal::install {

SystemdServiceInstaller::SystemdServiceInstaller(const std::optional<std::string> &kvp_string,
                                                 const std::optional<std::string> &only_save_path)
    : only_save_path_(only_save_path) {
  if (only_save_path_ && only_save_path_->empty()) {
    only_save_path_ = ".";
  }

  populate_defaults_();

  if (kvp_string && !kvp_string->empty()) {
    if (!parse_kvp_(*kvp_string)) {
      throw std::invalid_argument("invalid --systemd-service key:value format");
    }
    using_defaults_ = false;
  }
}

} // namespace neonsignal::install
