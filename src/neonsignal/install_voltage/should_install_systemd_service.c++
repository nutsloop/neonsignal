#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

bool install_voltage::should_install_systemd_service() const {
  return should_install_systemd_service_;
}

} // namespace neonsignal
