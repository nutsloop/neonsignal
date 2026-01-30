#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_url_hits_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Comma-separated list of URL paths that should set the mail cookie.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  /contact.html,/enroll.html\n\n"
      "{}\n"
      "  {} spin --mail-url-hits=/contact.html,/enroll.html\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-url-hits=<list>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_URL_HITS").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
