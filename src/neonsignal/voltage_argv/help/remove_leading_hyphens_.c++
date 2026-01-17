#include "neonsignal/voltage_argv/help.h++"

namespace neonsignal::voltage_argv {

void help::remove_leading_hyphens_() {
  while (!look_up_option_.empty() &&
         (look_up_option_.front() == '-' || look_up_option_.front() == '?')) {
    look_up_option_.erase(look_up_option_.begin());
  }
}

} // namespace neonsignal::voltage_argv
