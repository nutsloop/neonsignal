#include "neonsignal/voltage_argv/help.h++"

namespace neonsignal::voltage_argv {

std::string help::to_string() {
  Topic_ topic = stoenum_();
  return get_help_topic_(topic);
}

} // namespace neonsignal::voltage_argv
