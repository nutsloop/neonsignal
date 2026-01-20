#include "spin/api_handler.h++"

namespace neonsignal {

bool ApiHandler::auth_login_finish_headers(
    const std::shared_ptr<Http2Connection>& conn, std::uint32_t stream_id,
    const std::string& path, const std::string& method) {
  Http2Connection::StreamState st{};
  st.path = path;
  st.method = method;
  st.expect_body = true;
  st.is_auth_finish = true;
  conn->streams[stream_id] = std::move(st);
  return true;
}

} // namespace neonsignal
