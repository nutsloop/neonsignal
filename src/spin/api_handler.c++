#include "spin/api_handler.h++"
#include "spin/event_loop.h++"

namespace neonsignal {

ApiHandler::ApiHandler(EventLoop& loop, WebAuthnManager& auth,
                       const Router& router,
                       Database& db,
                       std::atomic<std::uint64_t>& served_files,
                       std::atomic<std::uint64_t>& page_views,
                       std::atomic<std::uint64_t>& event_clients,
                       std::atomic<bool>& redirect_ok)
    : loop_(loop), auth_(auth), router_(router), db_(db),
      served_files_(served_files), page_views_(page_views),
      event_clients_(event_clients),
      redirect_service_ok_(redirect_ok),
      codex_runner_(db) {}

} // namespace neonsignal
