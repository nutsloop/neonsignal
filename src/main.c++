#include "neonsignal/neonsignal.h++"
#include "neonsignal/logging.h++"

#include <exception>
#include <iostream>

int main() {
  try {
    neonsignal::install_thread_logging_prefix();
    neonsignal::Server server;
    server.run();
  } catch (const std::exception& ex) {
    std::cerr << "Fatal: " << ex.what() << '\n';
    return 1;
  }

  return 0;
}
