#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::mail_domains_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Comma-separated list of allowed domains for /api/mail requests.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  10.0.0.6:8889\n\n"
      "{}\n"
      "  {} spin --mail-domains=10.0.0.6:8889\n"
      "  {} spin --mail-domains=example.com,example.org\n",
      ansi("NAME").stylish().bold().str(), ansi("--mail-domains=<list>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_MAIL_DOMAINS").bright_cyan().str(),
      ansi("DEFAULT").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
