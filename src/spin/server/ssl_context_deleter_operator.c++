#include "spin/neonsignal.h++"

#include <openssl/ssl.h>

namespace neonsignal {

void Server::SSLContextDeleter::operator()(SSL_CTX * /*ctx*/) const {
  // SSL_CTX is now owned by CertManager, not by ssl_ctx_ unique_ptr
  // Do not free here - CertManager handles cleanup
}

} // namespace neonsignal
