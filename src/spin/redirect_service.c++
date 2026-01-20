#include "spin/redirect_service.h++"

#include <utility>

namespace neonsignal {

RedirectService::RedirectService(const int listen_port, std::string redirect_host,
                                 const int redirect_port, std::string acme_root)
    : listen_port_(listen_port), redirect_host_(std::move(redirect_host)),
      redirect_port_(redirect_port), acme_root_(std::move(acme_root)) {}

RedirectService::~RedirectService() { stop(); }

} // namespace neonsignal
