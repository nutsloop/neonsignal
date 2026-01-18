#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::www_root_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Root directory for serving static files.\n"
      "  Subdirectories are matched by SNI hostname (e.g., example.com/ for https://example.com).\n"
      "  The path can be relative or absolute.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  public\n\n"
      "{}\n"
      "  {} spin --www-root=./public\n"
      "  {} spin --www-root=/var/www/neonsignal\n",
      ansi("NAME").stylish().bold().str(), ansi("--www-root=<path>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_WWW_ROOT").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
