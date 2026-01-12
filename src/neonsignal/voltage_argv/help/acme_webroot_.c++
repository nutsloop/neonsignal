#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::acme_webroot_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Directory for ACME challenge files (for neonsignal_redirect).\n"
      "  Used for Let's Encrypt certificate validation.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  {} spin --acme-webroot=./acme-challenge\n"
      "  {} spin --acme-webroot=/var/www/acme\n",
      ansi("NAME").stylish().bold().str(), ansi("--acme-webroot=<path>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("ACME_WEBROOT").bright_cyan().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
