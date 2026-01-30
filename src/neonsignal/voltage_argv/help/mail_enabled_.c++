#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_enabled_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Enable or disable the /api/mail endpoint and mail cookie issuance.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  false\n\n"
      "{}\n"
      "  {} spin --mail-enabled=true\n"
      "  {} spin --mail-enabled=false\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-enabled=<bool>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_ENABLED").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
