#include "neonsignal/voltage_argv/help.h++"

namespace neonsignal::voltage_argv {

void help::set_option_list_() {
  option_list_ = {{"help", Topic_::help},
                  {"version", Topic_::version},
                  {"systemd", Topic_::systemd},
                  {"spin", Topic_::spin}};

  if (mode() == Mode::server) {
    option_list_.insert({{"threads", Topic_::threads},
                         {"host", Topic_::host},
                         {"port", Topic_::port},
                         {"webauthn-domain", Topic_::webauthn_domain},
                         {"webauthn-origin", Topic_::webauthn_origin},
                         {"db-path", Topic_::db_path},
                         {"www-root", Topic_::www_root},
                         {"certs-root", Topic_::certs_root},
                         {"working-dir", Topic_::working_dir}});
    return;
  }

  option_list_.insert({{"instances", Topic_::instances},
                       {"host", Topic_::host},
                       {"port", Topic_::port},
                       {"target-port", Topic_::target_port},
                       {"acme-webroot", Topic_::acme_webroot}});
}

} // namespace neonsignal::voltage_argv
