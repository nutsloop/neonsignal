#include "neonsignal/http2_listener_helpers.h++"

#include <fstream>
#include <sstream>
#include <string>

namespace neonsignal {

std::uint64_t read_rss_kb() {
  std::ifstream status("/proc/self/status");
  std::string line;
  while (std::getline(status, line)) {
    if (line.rfind("VmRSS:", 0) == 0) {
      std::istringstream iss(line);
      std::string key;
      std::uint64_t value_kb = 0;
      std::string unit;
      iss >> key >> value_kb >> unit;
      return value_kb;
    }
  }
  return 0;
}

} // namespace neonsignal
