#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::threads_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Number of worker threads for the event loop.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  3 (matches typical CPU count on Oracle Cloud Always Free tier)\n\n"
      "{}\n"
      "  {} spin --threads=4\n"
      "  {} spin --threads=8\n",
      ansi("NAME").stylish().bold().str(), ansi("--threads=<n>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_THREADS").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
