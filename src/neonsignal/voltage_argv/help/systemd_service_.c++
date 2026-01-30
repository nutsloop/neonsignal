#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::systemd_service_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Install systemd service files for neonsignal and neonsignal_redirect.\n"
      "  Requires root privileges. Can be used alone (defaults) or with key:value pairs.\n\n"
      "  ▲ Linux only when installing to /etc/systemd/system.\n"
      "  Use --only-save to write service files to a directory instead of /etc/systemd/system.\n"
      "  ▲ --only-save works on non-Linux platforms.\n\n"
      "{}\n"
      "  For neonsignal.service:\n"
      "    {}              User to run as (default: current user)\n"
      "    {}             Group to run as (default: current group)\n"
      "    {}       Working directory (default: current directory)\n"
      "    {}           Number of threads (default: 3)\n"
      "    {}              Listen host (default: 0.0.0.0)\n"
      "    {}              Listen port (default: 9443)\n"
      "    {}    WebAuthn RP ID domain (optional)\n"
      "    {}    WebAuthn origin URL (optional)\n"
      "    {}         Path to neonsignal binary (default: /usr/local/bin/neonsignal)\n\n"
      "    {}        Enable mail API (required if any mail keys are set)\n"
      "    {}       Mail allowed domains (comma-separated)\n"
      "    {}   Mail cookie name\n"
      "    {}   Mail cookie TTL in seconds\n"
      "    {}     Mail cookie URL hits\n"
      "    {}          Mail from addresses\n"
      "    {}      Mail extra recipients\n"
      "    {}        Mail command\n"
      "    {}     Mail allowed IP\n"
      "    {}        Mail save DB\n\n"
      "  For neonsignal_redirect.service:\n"
      "    {} Number of redirect workers (default: 3)\n"
      "    {}     Listen port (default: 9090)\n"
      "    {} Target HTTPS port (default: 443)\n"
      "    {}     Listen host (default: same as host)\n"
      "    {} Path to binary (default: /usr/local/bin/neonsignal_redirect)\n\n"
      "{}\n"
      "  {}\n"
      "  {}\n"
      "  {}\n"
      "  {}\n",
      ansi("NAME").stylish().bold().str(),
      ansi("--systemd-service[='key:value|...']").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(),
      ansi("PROPERTIES").stylish().bold().str(),
      ansi("user").bright_cyan().str(),
      ansi("group").bright_cyan().str(),
      ansi("working-dir").bright_cyan().str(),
      ansi("threads").bright_cyan().str(),
      ansi("host").bright_cyan().str(),
      ansi("port").bright_cyan().str(),
      ansi("webauthn-domain").bright_cyan().str(),
      ansi("webauthn-origin").bright_cyan().str(),
      ansi("exec-path").bright_cyan().str(),
      ansi("mail-enabled").bright_cyan().str(),
      ansi("mail-domains").bright_cyan().str(),
      ansi("mail-cookie-name").bright_cyan().str(),
      ansi("mail-cookie-ttl").bright_cyan().str(),
      ansi("mail-url-hits").bright_cyan().str(),
      ansi("mail-from").bright_cyan().str(),
      ansi("mail-to-extra").bright_cyan().str(),
      ansi("mail-command").bright_cyan().str(),
      ansi("mail-allowed-ip").bright_cyan().str(),
      ansi("mail-save-db").bright_cyan().str(),
      ansi("redirect-instances").bright_cyan().str(),
      ansi("redirect-port").bright_cyan().str(),
      ansi("redirect-target-port").bright_cyan().str(),
      ansi("redirect-host").bright_cyan().str(),
      ansi("redirect-exec-path").bright_cyan().str(),
      ansi("EXAMPLES").stylish().bold().str(),
      ansi("<binary> install --systemd-service").bright_yellow().str(),
      ansi("<binary> install --systemd-service='user:core|group:core'").bright_yellow().str(),
      ansi("<binary> install --systemd-service='user:www|threads:4|port:443'").bright_yellow().str(),
      ansi("<binary> install --systemd-service='mail-enabled:true|mail-domains:example.com|mail-from:noreply@example.com'").bright_yellow().str(),
      ansi("<binary> install --systemd-service --only-save=./systemd").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
