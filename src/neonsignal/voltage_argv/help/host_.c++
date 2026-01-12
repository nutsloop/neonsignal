#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::host_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  IPv4 address to bind the server to.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  0.0.0.0 (listen on all interfaces)\n\n"
      "{}\n"
      "  {} spin --host=127.0.0.1\n"
      "  {} spin --host=0.0.0.0\n",
      ansi("NAME").stylish().bold().str(), ansi("--host=<addr>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_HOST").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
