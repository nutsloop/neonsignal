#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_to_extra_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Comma-separated list of extra recipients that are always included.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  contact@example.com,support@example.com\n\n"
      "{}\n"
      "  {} spin --mail-to-extra=contact@example.com,support@example.com\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-to-extra=<list>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_TO_EXTRA").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
