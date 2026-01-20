#include "install/systemd_service_installer.h++"

#include <stdexcept>

namespace neonsignal::install {

SystemdServiceInstaller::SystemdServiceInstaller(const std::optional<std::string> &kvp_string) {
  populate_defaults_();

  if (kvp_string && !kvp_string->empty()) {
    if (!parse_kvp_(*kvp_string)) {
      throw std::invalid_argument("invalid --systemd-service key:value format");
    }
    using_defaults_ = false;
  }
}

} // namespace neonsignal::install
