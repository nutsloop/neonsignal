#include "neonsignal/voltage_argv/help.h++"

#include <format>

namespace neonsignal::voltage_argv {

std::string help::get_version_() const {
  return std::format("{}.{}.{}", version_info_[0], version_info_[1], version_info_[2]);
}

} // namespace neonsignal::voltage_argv
