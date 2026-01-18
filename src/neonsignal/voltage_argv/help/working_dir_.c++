#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::working_dir_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Change the process working directory before resolving other paths.\n"
      "  This affects relative paths like ./public, ./certs, ./data.\n"
      "  The path must exist, be a directory, and be writable.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  (none - current directory is used if not specified)\n\n"
      "{}\n"
      "  {} spin --working-dir=/opt/neonsignal\n"
      "  {} spin --working-dir=../deployment\n",
      ansi("NAME").stylish().bold().str(), ansi("--working-dir=<path>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_WORKING_DIR").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
