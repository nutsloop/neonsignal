#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::webauthn_origin_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  WebAuthn origin URL - full site URL for passwordless login verification.\n"
      "  Must match the URL users see in their browser.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  {} spin --webauthn-origin=https://example.com\n"
      "  {} spin --webauthn-origin=https://app.example.com:8443\n",
      ansi("NAME").stylish().bold().str(), ansi("--webauthn-origin=<url>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_WEBAUTHN_ORIGIN").bright_cyan().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
