#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::repo_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Specifies the git repository URL to clone. This option is required for the\n"
      "  install command. Supported URL formats include:\n"
      "    - https://github.com/user/repo.git\n"
      "    - git@github.com:user/repo.git\n"
      "    - git://github.com/user/repo.git\n"
      "    - ssh://git@github.com/user/repo.git\n\n"
      "{}\n"
      "  {} install --repo=https://github.com/user/repo.git\n"
      "  {} install --repo=git@github.com:user/app.git\n",
      ansi("NAME").stylish().bold().str(), ansi("--repo").bright_cyan().str(),
      ansi("DESCRIPTION").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
