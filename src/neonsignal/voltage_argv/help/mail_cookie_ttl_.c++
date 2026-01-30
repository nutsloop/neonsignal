#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_cookie_ttl_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Mail cookie TTL in seconds.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  900\n\n"
      "{}\n"
      "  {} spin --mail-cookie-ttl=900\n"
      "  {} spin --mail-cookie-ttl=1200\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-cookie-ttl=<seconds>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_COOKIE_TTL").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
