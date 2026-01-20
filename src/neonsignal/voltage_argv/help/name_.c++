#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::install_name_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Specifies a custom directory name for the cloned repository. If not provided,\n"
      "  the repository name will be extracted from the URL and used as the directory name.\n"
      "  The name cannot contain path separators (/ or \\) or be '.' or '..'.\n\n"
      "{}\n"
      "  {} install --repo=https://github.com/user/repo.git --name=myapp\n"
      "  {} install --repo=git@github.com:user/frontend.git --name=web\n",
      ansi("NAME").stylish().bold().str(), ansi("--name").bright_cyan().str(),
      ansi("DESCRIPTION").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
