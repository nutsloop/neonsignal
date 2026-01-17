#include "neonsignal/voltage_argv/help.h++"

namespace neonsignal::voltage_argv {

help::Topic_ help::stoenum_() {
  auto it = option_list_.find(look_up_option_);
  if (it != option_list_.end()) {
    return it->second;
  }
  return Topic_::unknown;
}

} // namespace neonsignal::voltage_argv
