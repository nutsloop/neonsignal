#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<unsigned long long> &redirect_voltage::port() const { return port_; }

} // namespace neonsignal
