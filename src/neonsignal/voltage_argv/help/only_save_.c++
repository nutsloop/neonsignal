#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::only_save_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Save generated systemd service files to a directory instead of installing\n"
      "  them under /etc/systemd/system. This option is only valid with --systemd-service.\n"
      "  When the path is omitted, files are saved in the current directory.\n"
      "  Outputs neonsignal.service and neonsignal_redirect.service.\n\n"
      "{}\n"
      "  {} install --systemd-service --only-save\n"
      "  {} install --systemd-service --only-save=./systemd\n",
      ansi("NAME").stylish().bold().str(), ansi("--only-save[=<path>]").bright_cyan().str(),
      ansi("DESCRIPTION").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
