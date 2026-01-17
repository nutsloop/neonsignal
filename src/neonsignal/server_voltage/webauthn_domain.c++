#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<std::string> &server_voltage::webauthn_domain() const {
  return webauthn_domain_;
}

} // namespace neonsignal
