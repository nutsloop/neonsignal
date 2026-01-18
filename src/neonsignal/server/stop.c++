#include "neonsignal/neonsignal.h++"
#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener.h++"

namespace neonsignal {

void Server::stop() {
  // Stop accepting new connections first
  if (listener_) {
    listener_->shutdown_graceful();
  }

  if (loop_) {
    // Use graceful shutdown to drain remaining connections (3s timeout)
    loop_->shutdown_graceful();
  }
}

} // namespace neonsignal
