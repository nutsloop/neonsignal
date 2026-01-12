#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<unsigned long long> &redirect_voltage::target_port() const {
  return target_port_;
}

} // namespace neonsignal
