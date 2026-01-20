#include "install/systemd_service_installer.h++"

#include <format>

namespace neonsignal::install {

std::string SystemdServiceInstaller::generate_neonsignal_service_() const {
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
      "ExecStart={} --systemd\n"
      "Restart=on-failure\n"
      "AmbientCapabilities=CAP_NET_BIND_SERVICE\n"
      "LimitNOFILE=65536\n"
      "\n"
      "[Install]\n"
      "WantedBy=multi-user.target\n",
      config_.user, config_.group, config_.working_dir, config_.threads, config_.host, config_.port,
      webauthn_domain_env, webauthn_origin_env, config_.exec_path);
}

} // namespace neonsignal::install
