#include "neonsignal/voltage_argv/help.h++"

#include <utility>

namespace neonsignal::voltage_argv {

help::help(std::string look_up_option, nutsloop::args::version_t version)
    : look_up_option_(std::move(look_up_option)), version_info_(version) {
  set_option_list_();
  remove_leading_hyphens_();
}

} // namespace neonsignal::voltage_argv
