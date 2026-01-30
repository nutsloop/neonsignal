#include "neonsignal/voltage_argv/help.h++"

namespace neonsignal::voltage_argv {

void help::set_option_list_() {
  option_list_ = {{"help", Topic_::help},
                  {"version", Topic_::version},
                  {"systemd", Topic_::systemd},
                  {"spin", Topic_::spin},
                  {"install", Topic_::install}};

  if (mode() == Mode::server) {
    option_list_.insert({{"threads", Topic_::threads},
                         {"host", Topic_::host},
                         {"port", Topic_::port},
                         {"webauthn-domain", Topic_::webauthn_domain},
                         {"webauthn-origin", Topic_::webauthn_origin},
                         {"db-path", Topic_::db_path},
                         {"www-root", Topic_::www_root},
                         {"certs-root", Topic_::certs_root},
                         {"working-dir", Topic_::working_dir},
                         {"mail-enabled", Topic_::mail_enabled},
                         {"mail-domains", Topic_::mail_domains},
                         {"mail-cookie-name", Topic_::mail_cookie_name},
                         {"mail-cookie-ttl", Topic_::mail_cookie_ttl},
                         {"mail-url-hits", Topic_::mail_url_hits},
                         {"mail-from", Topic_::mail_from},
                         {"mail-to-extra", Topic_::mail_to_extra},
                         {"mail-command", Topic_::mail_command},
                         {"mail-allowed-ip", Topic_::mail_allowed_ip},
                         {"mail-save-db", Topic_::mail_save_db},
                         {"repo", Topic_::repo},
                         {"name", Topic_::install_name},
                         {"branch", Topic_::branch},
                         {"systemd-service", Topic_::systemd_service},
                         {"only-save", Topic_::only_save}});
    return;
  }

  option_list_.insert({{"instances", Topic_::instances},
                       {"host", Topic_::host},
                       {"port", Topic_::port},
                       {"target-port", Topic_::target_port},
                       {"acme-webroot", Topic_::acme_webroot}});
}

} // namespace neonsignal::voltage_argv
