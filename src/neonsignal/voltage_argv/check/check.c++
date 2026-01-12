#include "neonsignal/voltage_argv/check.h++"

#include <utility>

namespace neonsignal::voltage_argv {

check::check(args_key_value_t_ value) : value_(std::move(value)) {}

} // namespace neonsignal::voltage_argv
