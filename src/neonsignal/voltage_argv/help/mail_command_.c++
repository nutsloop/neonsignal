#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_command_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Mail command to execute (for example: mail, msmtp).\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  mail\n\n"
      "{}\n"
      "  {} spin --mail-command=mail\n"
      "  {} spin --mail-command=msmtp\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-command=<cmd>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_COMMAND").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
