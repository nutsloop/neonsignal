#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::instances_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Number of redirect service instances (for neonsignal_redirect).\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  2\n\n"
      "{}\n"
      "  {} spin --instances=3\n"
      "  {} spin --instances=8\n",
      ansi("NAME").stylish().bold().str(), ansi("--instances=<n>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("REDIRECT_INSTANCES").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
