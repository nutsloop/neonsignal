#include "neonsignal/event_loop.h++"
#include "neonsignal/neonsignal.h++"

namespace neonsignal {

void Server::stop() {
  if (loop_) {
    // Use graceful shutdown to drain connections (30s timeout)
    loop_->shutdown_graceful();
  }
}

} // namespace neonsignal
