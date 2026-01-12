#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<unsigned long long> &redirect_voltage::instances() const { return instances_; }

} // namespace neonsignal
