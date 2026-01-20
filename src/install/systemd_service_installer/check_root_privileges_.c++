#include "install/systemd_service_installer.h++"

#include <unistd.h>

namespace neonsignal::install {

bool SystemdServiceInstaller::check_root_privileges_() const { return geteuid() == 0; }

} // namespace neonsignal::install
