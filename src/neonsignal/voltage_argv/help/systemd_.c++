#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::systemd_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Boolean flag to skip the 'spin' command requirement when running as a systemd service.\n"
      "  This flag takes no value.\n\n"
      "{}\n"
      "  {} --systemd --host=0.0.0.0 --port=9443\n",
      ansi("NAME").stylish().bold().str(), ansi("--systemd").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
