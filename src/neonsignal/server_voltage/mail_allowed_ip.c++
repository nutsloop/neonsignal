#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<std::string> &server_voltage::mail_allowed_ip() const {
  return mail_allowed_ip_;
}

} // namespace neonsignal
