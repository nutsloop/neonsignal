#include "install/systemd_service_installer.h++"

#include <format>

namespace neonsignal::install {

std::string SystemdServiceInstaller::generate_neonsignal_service_() const {
  std::string mail_env;
  if (config_.mail_enabled) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_ENABLED={}\n",
                            *config_.mail_enabled ? "true" : "false");
  }
  if (config_.mail_domains) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_DOMAINS={}\n", *config_.mail_domains);
  }
  if (config_.mail_cookie_name) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_COOKIE_NAME={}\n",
                            *config_.mail_cookie_name);
  }
  if (config_.mail_cookie_ttl) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_COOKIE_TTL={}\n",
                            *config_.mail_cookie_ttl);
  }
  if (config_.mail_url_hits) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_URL_HITS={}\n",
                            *config_.mail_url_hits);
  }
  if (config_.mail_from) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_FROM={}\n", *config_.mail_from);
  }
  if (config_.mail_to_extra) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_TO_EXTRA={}\n",
                            *config_.mail_to_extra);
  }
  if (config_.mail_command) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_COMMAND={}\n", *config_.mail_command);
  }
  if (config_.mail_allowed_ip) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_ALLOWED_IP={}\n",
                            *config_.mail_allowed_ip);
  }
  if (config_.mail_save_db) {
    mail_env += std::format("Environment=NEONSIGNAL_MAIL_SAVE_DB={}\n",
                            *config_.mail_save_db ? "true" : "false");
  }
  std::string webauthn_domain_env;
  if (config_.webauthn_domain) {
    webauthn_domain_env = std::format("Environment=NEONSIGNAL_WEBAUTHN_DOMAIN={}\n", *config_.webauthn_domain);
  }

  std::string webauthn_origin_env;
  if (config_.webauthn_origin) {
    webauthn_origin_env = std::format("Environment=NEONSIGNAL_WEBAUTHN_ORIGIN={}\n", *config_.webauthn_origin);
  }

  return std::format(
      "[Unit]\n"
      "Description=neonsignal HTTP/2 server\n"
      "After=network-online.target\n"
      "Wants=network-online.target\n"
      "\n"
      "[Service]\n"
      "User={}\n"
      "Group={}\n"
      "WorkingDirectory={}\n"
      "Environment=NEONSIGNAL_THREADS={}\n"
      "Environment=NEONSIGNAL_HOST={}\n"
      "Environment=NEONSIGNAL_PORT={}\n"
      "{}"
      "{}"
      "{}"
      "ExecStart={} --systemd\n"
      "Restart=on-failure\n"
      "AmbientCapabilities=CAP_NET_BIND_SERVICE\n"
      "LimitNOFILE=65536\n"
      "\n"
      "[Install]\n"
      "WantedBy=multi-user.target\n",
      config_.user, config_.group, config_.working_dir, config_.threads, config_.host, config_.port,
      webauthn_domain_env, webauthn_origin_env, mail_env, config_.exec_path);
}

} // namespace neonsignal::install
