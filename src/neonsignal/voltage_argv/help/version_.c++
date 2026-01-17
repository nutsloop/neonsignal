#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::version_() const {
  using nutsloop::ansi;

  return std::format("{} {}\n", ansi("NeonSignal version").stylish().bold().str(),
                     ansi(get_version_()).bright_cyan().str());
}

} // namespace neonsignal::voltage_argv
