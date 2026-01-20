#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::unknown_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n  Topic '{}' not found.\n\n{}\n  {} --help\n", ansi("✗").bright_red().bold().str(),
      ansi(look_up_option_).bright_yellow().str(), ansi("▸").bright_cyan().bold().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
