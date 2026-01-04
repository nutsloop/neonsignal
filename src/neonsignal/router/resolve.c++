#include "neonsignal/router.h++"

#include "neonsignal/routes.h++"

#include <filesystem>
#include <string>
#include <string_view>

namespace neonsignal {

RouteResult Router::resolve(std::string_view path,
                            const std::filesystem::path &document_root) const {
  RouteResult res;

  std::string clean(path);

  // Strip query string (?key=value&...)
  if (auto const query_pos = clean.find('?'); query_pos != std::string::npos) {
    clean.erase(query_pos);
  }

  if (clean.empty() || clean[0] != '/') {
    clean.insert(clean.begin(), '/');
  }
  if (clean == routes::pages::kHome) {
    clean = std::string(routes::default_document());
  }
  if (clean.find("..") != std::string::npos) {
    return res;
  }

  std::filesystem::path full = document_root / clean.substr(1);
  if (std::filesystem::is_directory(full)) {
    full /= std::string(routes::default_document()).substr(1);
  }

  if (std::filesystem::exists(full) && std::filesystem::is_regular_file(full)) {
    res.file = full;
    res.found = true;
  }

  return res;
}

RouteResult Router::resolve(std::string_view path) const { return resolve(path, public_root_); }

} // namespace neonsignal
