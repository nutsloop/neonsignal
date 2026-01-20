#include "install/systemd_service_installer.h++"

#include <format>

namespace neonsignal::install {

std::string SystemdServiceInstaller::generate_redirect_service_() const {
  return std::format(
      "[Unit]\n"
      "Description=neonsignal redirector (HTTP -> HTTPS)\n"
      "After=network-online.target\n"
      "Wants=network-online.target\n"
      "\n"
      "[Service]\n"
      "User={}\n"
      "Group={}\n"
      "WorkingDirectory={}\n"
      "Environment=REDIRECT_INSTANCES={}\n"
      "Environment=REDIRECT_PORT={}\n"
      "Environment=REDIRECT_TARGET_PORT={}\n"
      "Environment=REDIRECT_HOST={}\n"
      "ExecStart={} --systemd\n"
      "Restart=on-failure\n"
      "AmbientCapabilities=CAP_NET_BIND_SERVICE\n"
      "LimitNOFILE=65536\n"
      "\n"
      "[Install]\n"
      "WantedBy=multi-user.target\n",
      config_.user, config_.group, config_.working_dir, config_.redirect_instances, config_.redirect_port,
      config_.redirect_target_port, config_.redirect_host, config_.redirect_exec_path);
}

} // namespace neonsignal::install
