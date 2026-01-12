#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::port_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Port number to listen on (1-65535).\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  9443 (non-privileged HTTPS port)\n\n"
      "{}\n"
      "  {} spin --port=8443\n"
      "  {} spin --port=9443\n",
      ansi("NAME").stylish().bold().str(), ansi("--port=<n>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_PORT").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
