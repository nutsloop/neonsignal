#include "neonsignal/voltage_argv.h++"

#include <cctype>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

namespace neonsignal {

namespace {

[[noreturn]] void throw_invalid(const std::string &message) {
  throw std::invalid_argument(message);
}

} // namespace

void server_voltage::validate_dash_format_(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg.size() >= 2 && arg[0] == '-' && arg[1] != '-' &&
        std::isalpha(static_cast<unsigned char>(arg[1]))) {
      // Single dash with alphabetic char (e.g., -port, -host)
      // Allow single-char flags like -v, -h but reject multi-char like -port
      if (arg.size() > 2 && arg[2] != '=') {
        throw_invalid(
            std::format("invalid flag format: '{}'\n  Use '--{}' instead (double dash required)",
                        std::string(arg), std::string(arg.substr(1))));
      }
    }
  }
}

void redirect_voltage::validate_dash_format_(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg.size() >= 2 && arg[0] == '-' && arg[1] != '-' &&
        std::isalpha(static_cast<unsigned char>(arg[1]))) {
      // Single dash with alphabetic char (e.g., -port, -host)
      // Allow single-char flags like -v, -h but reject multi-char like -port
      if (arg.size() > 2 && arg[2] != '=') {
        throw_invalid(
            std::format("invalid flag format: '{}'\n  Use '--{}' instead (double dash required)",
                        std::string(arg), std::string(arg.substr(1))));
      }
    }
  }
}

} // namespace neonsignal
