#include "spin/redirect_service.h++"

#include <sstream>

namespace neonsignal {

void RedirectService::send_redirect_(Connection &conn, const std::string &host,
                                     const std::string &path) const {
  std::ostringstream location;
  location << "https://" << host;
  if (redirect_port_ != 443) {
    location << ':' << redirect_port_;
  }
  if (!path.empty()) {
    location << path;
  } else {
    location << '/';
  }

  const std::string target = location.str();
  const std::string body = "Redirecting to " + target + "\n";

  std::ostringstream response;
  response << "HTTP/1.1 308 Permanent Redirect\r\n"
           << "Location: " << target << "\r\n"
           << "Content-Type: text/plain\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n\r\n"
           << body;

  conn.write_buffer = response.str();
  conn.write_ready = true;
}

} // namespace neonsignal
