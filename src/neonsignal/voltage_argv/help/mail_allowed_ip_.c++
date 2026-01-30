#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_allowed_ip_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Optional IP address whitelist for /api/mail (empty means allow all).\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  (empty)\n\n"
      "{}\n"
      "  {} spin --mail-allowed-ip=127.0.0.1\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-allowed-ip=<ip>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_ALLOWED_IP").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
