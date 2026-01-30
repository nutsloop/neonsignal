#include "neonsignal/voltage_argv.h++"

namespace neonsignal {

const std::optional<unsigned long long> &server_voltage::mail_cookie_ttl() const {
  return mail_cookie_ttl_;
}

} // namespace neonsignal
