#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_save_db_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Enable or disable storing mail submissions in the database.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  true\n\n"
      "{}\n"
      "  {} spin --mail-save-db=true\n"
      "  {} spin --mail-save-db=false\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-save-db=<bool>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_SAVE_DB").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
