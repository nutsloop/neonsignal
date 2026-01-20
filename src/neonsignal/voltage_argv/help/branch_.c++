#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::branch_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Specifies a specific branch to clone. If not provided, the repository's\n"
      "  default branch will be used. The branch name cannot start with '-',\n"
      "  contain '..', or end with '/'.\n\n"
      "{}\n"
      "  {} install --repo=https://github.com/user/repo.git --branch=develop\n"
      "  {} install --repo=git@github.com:user/app.git --branch=v2.0\n"
      "  {} install --repo=https://github.com/user/site.git --branch=feature/new-ui\n",
      ansi("NAME").stylish().bold().str(), ansi("--branch").bright_cyan().str(),
      ansi("DESCRIPTION").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
