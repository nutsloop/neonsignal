#include "neonsignal/voltage_argv/help.h++"

namespace neonsignal::voltage_argv {

void help::set_option_list_() {
  option_list_ = {{"help", Topic_::help},
                  {"version", Topic_::version},
                  {"threads", Topic_::threads},
                  {"host", Topic_::host},
                  {"port", Topic_::port},
                  {"webauthn-domain", Topic_::webauthn_domain},
                  {"webauthn-origin", Topic_::webauthn_origin},
                  {"db-path", Topic_::db_path},
                  {"systemd", Topic_::systemd},
                  {"instances", Topic_::instances},
                  {"target-port", Topic_::target_port},
                  {"acme-webroot", Topic_::acme_webroot},
                  {"spin", Topic_::spin}};
}

} // namespace neonsignal::voltage_argv
