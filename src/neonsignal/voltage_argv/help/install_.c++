#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::install_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Clone a git repository into the www-root directory using shallow clone (--depth=1).\n"
      "  The repository will be cloned to the www-root directory with the repository name\n"
      "  as the directory name, unless --name is specified.\n\n"
      "{}\n"
      "  --repo=<url>      Git repository URL (required unless --systemd-service is used)\n"
      "  --www-root=<path> Target directory for clone (default: ./public)\n"
      "  --name=<name>     Custom directory name for the clone\n"
      "  --branch=<branch> Specific branch to clone\n"
      "  --systemd-service[=kvp]\n"
      "                   Install systemd service files (defaults when omitted)\n"
      "                   ▲ Linux only; exits with error on other platforms\n"
      "                   See --help=systemd-service for KVP details\n\n"
      "{}\n"
      "  {}\n"
      "    {}\n"
      "  {}\n"
      "    {}\n"
      "    {}\n"
      "    {}\n"
      "    {}\n\n"
      "{}\n"
      "  {} install --repo=https://github.com/user/repo.git\n"
      "  {} install --repo=git@github.com:user/app.git --name=myapp\n"
      "  {} install --repo=https://github.com/user/site.git --www-root=/var/www\n"
      "  {} install --repo=https://github.com/user/repo.git --branch=develop\n"
      "  {} install --systemd-service='user:core|group:core'\n",
      ansi("NAME").stylish().bold().str(), ansi("install").bright_cyan().str(),
      ansi("DESCRIPTION").stylish().bold().str(),
      ansi("OPTIONS").stylish().bold().str(),
      ansi("SWITCH SETS").stylish().bold().str(),
      ansi("  <binary> install").bright_yellow().str(),
      ansi("--systemd-service").bright_green().str(),
      ansi("  <binary> install").bright_yellow().str(),
      ansi("--repo").bright_green().str(), ansi("--www-root").bright_green().str(),
      ansi("--name").bright_green().str(), ansi("--branch").bright_green().str(),
      ansi("EXAMPLES").stylish().bold().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
