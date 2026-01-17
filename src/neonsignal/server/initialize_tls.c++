#include "neonsignal/neonsignal.h++"
#include "neonsignal/cert_manager.h++"

#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>

namespace {

void ensure_openssl_initialized() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
  initialized = true;
}

// SNI callback - called during TLS handshake to select the correct SSL_CTX
int sni_callback(SSL *ssl, int * /*alert*/, void *arg) {
  auto *cert_mgr = static_cast<neonsignal::CertManager *>(arg);

  const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  if (!servername) {
    // No SNI extension - use default cert (already set)
    return SSL_TLSEXT_ERR_OK;
  }

  SSL_CTX *ctx = cert_mgr->get_context(servername);
  if (!ctx) {
    // No matching cert - keep using current (default) context
    return SSL_TLSEXT_ERR_OK;
  }

  // Switch to the domain-specific SSL_CTX
  SSL_set_SSL_CTX(ssl, ctx);

  return SSL_TLSEXT_ERR_OK;
}

} // namespace

namespace neonsignal {

void Server::initialize_tls() {
  ensure_openssl_initialized();

  // Initialize the CertManager and load all certificates
  cert_manager_ = std::make_unique<CertManager>(config_.certs_root);
  if (!cert_manager_->initialize()) {
    throw std::runtime_error(
        "failed to initialize certificate manager from " + config_.certs_root);
  }

  // Get the default context from CertManager
  SSL_CTX *default_ctx = cert_manager_->get_default_context();
  if (!default_ctx) {
    throw std::runtime_error("no default certificate available");
  }

  // Store a non-owning pointer (CertManager owns the SSL_CTX objects)
  // We create a dummy unique_ptr that doesn't actually free the context
  ssl_ctx_ = std::unique_ptr<SSL_CTX, SSLContextDeleter>(default_ctx);

  // Register SNI callback on the default context
  SSL_CTX_set_tlsext_servername_callback(default_ctx, sni_callback);
  SSL_CTX_set_tlsext_servername_arg(default_ctx, cert_manager_.get());

  // Log loaded certificates
  std::cerr << "neonsignal->TLS certificates loaded:\n";
  for (const auto &cert : cert_manager_->list_certificates()) {
    std::cerr << "  " << cert << '\n';
  }

  // Warn about expiring certificates
  auto expiring = cert_manager_->expiring_soon(30);
  for (const auto &cert : expiring) {
    std::cerr << "  WARNING: " << cert << '\n';
  }
}

} // namespace neonsignal
