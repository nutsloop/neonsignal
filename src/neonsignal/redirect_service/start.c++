#include "neonsignal/redirect_service.h++"

#include <format>
#include <iostream>

namespace neonsignal {

void RedirectService::start() {
  if (running_.exchange(true)) {
    return;
  }
  setup_listener_();
  std::cerr << std::format("@{}:{}", redirect_host_, listen_port_) << std::endl;
  loop_.run();
}

} // namespace neonsignal
