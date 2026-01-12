#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<std::string> &server_voltage::webauthn_origin() const {
  return webauthn_origin_;
}

} // namespace neonsignal
