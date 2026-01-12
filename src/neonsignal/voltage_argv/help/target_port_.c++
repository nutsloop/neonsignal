#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::target_port_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  HTTPS redirect target port (for neonsignal_redirect).\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  443\n\n"
      "{}\n"
      "  {} spin --target-port=443\n"
      "  {} spin --target-port=9443\n",
      ansi("NAME").stylish().bold().str(), ansi("--target-port=<n>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("REDIRECT_TARGET_PORT").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
