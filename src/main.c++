#include "neonsignal/logging.h++"
#include "neonsignal/neonsignal.h++"

#include <exception>
#include <iostream>
#include <ostream>

int main() {
  try {
    neonsignal::install_thread_logging_prefix();
    neonsignal::Server server;
    server.run();
  } catch (const std::exception &ex) {
    std::cerr << "Fatal: " << ex.what() << '\n';
    return 1;
  }

  return 0;
}
