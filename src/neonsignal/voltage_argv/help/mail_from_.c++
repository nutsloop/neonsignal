#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_from_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Comma-separated list of sender addresses. The first entry is used as the From header.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  noreply@example.com\n\n"
      "{}\n"
      "  {} spin --mail-from=noreply@example.com\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-from=<list>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_FROM").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
