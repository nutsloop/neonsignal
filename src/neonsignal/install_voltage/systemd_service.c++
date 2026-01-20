#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<std::string> &install_voltage::systemd_service() const {
  return systemd_service_;
}

} // namespace neonsignal
