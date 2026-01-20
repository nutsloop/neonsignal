#pragma once

#include "neonsignal/voltage_argv.h++"

#include <optional>
#include <string>

namespace neonsignal::install {

class InstallCommand {
public:
  explicit InstallCommand(const install_voltage &config);

  [[nodiscard]] int run();

private:
  std::string repo_url_;
  std::string www_root_;
  std::optional<std::string> target_name_;
  std::optional<std::string> branch_;

  [[nodiscard]] bool validate_repo_url_() const;
  [[nodiscard]] bool clone_repository_();
  [[nodiscard]] bool verify_clone_() const;
};

} // namespace neonsignal::install
