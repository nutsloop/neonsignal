#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<std::string> &server_voltage::mail_to_extra() const {
  return mail_to_extra_;
}

} // namespace neonsignal
