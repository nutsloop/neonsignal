#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_cookie_name_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Cookie name used for the mail anti-spam token.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  neon-mail\n\n"
      "{}\n"
      "  {} spin --mail-cookie-name=neon-mail\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-cookie-name=<name>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_COOKIE_NAME").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
