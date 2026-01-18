#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::certs_root_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Root directory for TLS certificates.\n"
      "  Subdirectories are matched by SNI hostname (e.g., example.com/ for https://example.com).\n"
      "  Each subdirectory should contain fullchain.pem and privkey.pem files.\n"
      "  The path can be relative or absolute.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  certs\n\n"
      "{}\n"
      "  {} spin --certs-root=./certs\n"
      "  {} spin --certs-root=/etc/neonsignal/certs\n",
      ansi("NAME").stylish().bold().str(), ansi("--certs-root=<path>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_CERTS_ROOT").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
