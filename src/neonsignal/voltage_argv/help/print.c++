#include "neonsignal/voltage_argv/help.h++"

#include <iostream>

namespace neonsignal::voltage_argv {

int help::print() {
  std::cout << to_string();
  return 0;
}

} // namespace neonsignal::voltage_argv
