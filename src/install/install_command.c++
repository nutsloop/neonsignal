#include "install/install_command.h++"

#include <cstdlib>
#include <filesystem>
#include <string>

namespace neonsignal::install {

InstallCommand::InstallCommand(const install_voltage &config)
    : repo_url_(*config.repo()),
      www_root_(config.www_root().value_or("./public")),
      target_name_(config.name()),
      branch_(config.branch()) {
}

} // namespace neonsignal::install
