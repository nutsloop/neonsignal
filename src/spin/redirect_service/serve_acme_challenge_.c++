#include "spin/redirect_service.h++"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace neonsignal {

bool RedirectService::serve_acme_challenge_(Connection &conn, const std::string &path) const {

  std::filesystem::path full = std::filesystem::path(acme_root_) / path.substr(1);
  if (std::error_code ec;
      !std::filesystem::exists(full, ec) || !std::filesystem::is_regular_file(full, ec)) {
    return false;
  }

  std::ifstream in(full, std::ios::binary);
  if (!in) {
    return false;
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  std::string body = ss.str();

  std::ostringstream resp;
  resp << "HTTP/1.1 200 OK\r\n"
       << "Content-Type: text/plain\r\n"
       << "Content-Length: " << body.size() << "\r\n"
       << "Connection: close\r\n\r\n"
       << body;
  conn.write_buffer = resp.str();
  conn.write_ready = true;
  return true;
}

} // namespace neonsignal
