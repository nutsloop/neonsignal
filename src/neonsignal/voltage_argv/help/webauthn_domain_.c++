#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::webauthn_domain_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  WebAuthn Relying Party ID - your domain for passwordless authentication.\n"
      "  This identifies your application to the authenticator.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  {} spin --webauthn-domain=example.com\n"
      "  {} spin --webauthn-domain=app.example.com\n",
      ansi("NAME").stylish().bold().str(), ansi("--webauthn-domain=<id>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_WEBAUTHN_DOMAIN").bright_cyan().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
