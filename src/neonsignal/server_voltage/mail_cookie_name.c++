#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<std::string> &server_voltage::mail_cookie_name() const {
  return mail_cookie_name_;
}

} // namespace neonsignal
