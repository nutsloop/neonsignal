#include "neonsignal/cert_manager.h++"

namespace neonsignal {

CertManager::CertManager(std::filesystem::path certs_root) : certs_root_(std::move(certs_root)) {}

} // namespace neonsignal
