#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::spin_() const {
  using nutsloop::ansi;

  if (mode() == Mode::redirect) {
    return std::format(
        "{}\n"
        "  {}\n\n"
        "{}\n"
        "  Start the redirector. This command is required unless --systemd flag is used.\n\n"
        "{}\n"
        "  {} spin\n"
        "  {} spin --port=9090 --target-port=443\n"
        "  {} spin --instances=3 --host=0.0.0.0\n",
        ansi("NAME").stylish().bold().str(), ansi("spin").bright_cyan().str(),
        ansi("DESCRIPTION").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
        ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str(),
        ansi("  <binary>").bright_yellow().str());
  }

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Start the server. This command is required unless --systemd flag is used.\n\n"
      "{}\n"
      "  {} spin\n"
      "  {} spin --host=0.0.0.0 --port=9443\n"
      "  {} spin --threads=4 --webauthn-domain=example.com\n",
      ansi("NAME").stylish().bold().str(), ansi("spin").bright_cyan().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
