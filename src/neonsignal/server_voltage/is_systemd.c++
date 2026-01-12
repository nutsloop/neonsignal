#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

bool server_voltage::is_systemd() const { return systemd_; }

} // namespace neonsignal
