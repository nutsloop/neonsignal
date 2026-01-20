#include "spin/neonsignal.h++"

namespace neonsignal {

void Server::configure_credentials() {
  // Certificate loading is now handled by CertManager in initialize_tls()
  // This function is kept for backward compatibility
  if (!ssl_ctx_) {
    initialize_tls();
  }
}

} // namespace neonsignal
