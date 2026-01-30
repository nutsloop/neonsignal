#include "neonsignal/voltage_argv/help.h++"

namespace neonsignal::voltage_argv {

std::string help::get_help_topic_(Topic_ topic) const {
  switch (topic) {
  case Topic_::help:
    return help_();
  case Topic_::version:
    return version_();
  case Topic_::threads:
    return threads_();
  case Topic_::host:
    return host_();
  case Topic_::port:
    return port_();
  case Topic_::webauthn_domain:
    return webauthn_domain_();
  case Topic_::webauthn_origin:
    return webauthn_origin_();
  case Topic_::db_path:
    return db_path_();
  case Topic_::www_root:
    return www_root_();
  case Topic_::certs_root:
    return certs_root_();
  case Topic_::working_dir:
    return working_dir_();
  case Topic_::mail_enabled:
    return mail_enabled_();
  case Topic_::mail_domains:
    return mail_domains_();
  case Topic_::mail_cookie_name:
    return mail_cookie_name_();
  case Topic_::mail_cookie_ttl:
    return mail_cookie_ttl_();
  case Topic_::mail_url_hits:
    return mail_url_hits_();
  case Topic_::mail_from:
    return mail_from_();
  case Topic_::mail_to_extra:
    return mail_to_extra_();
  case Topic_::mail_command:
    return mail_command_();
  case Topic_::mail_allowed_ip:
    return mail_allowed_ip_();
  case Topic_::mail_save_db:
    return mail_save_db_();
  case Topic_::systemd:
    return systemd_();
  case Topic_::instances:
    return instances_();
  case Topic_::target_port:
    return target_port_();
  case Topic_::acme_webroot:
    return acme_webroot_();
  case Topic_::repo:
    return repo_();
  case Topic_::install_name:
    return install_name_();
  case Topic_::branch:
    return branch_();
  case Topic_::systemd_service:
    return systemd_service_();
  case Topic_::only_save:
    return only_save_();
  case Topic_::spin:
    return spin_();
  case Topic_::install:
    return install_();
  case Topic_::unknown:
    return unknown_();
  }
  return unknown_();
}

} // namespace neonsignal::voltage_argv
