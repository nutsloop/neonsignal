#include "spin/http2_listener_helpers.h++"
#include "spin/static_cache.h++"

#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace neonsignal {

StaticResult load_static(std::string_view path, const Router& router,
                         StaticFileCache* cache) {
  StaticResult res;

  // Check cache first if available
  if (cache) {
    auto cached = cache->get(path);
    if (cached) {
      res.status = 200;
      res.content_type = cached->mime_type;
      res.body = cached->content;
      return res;
    }
  }

  // Cache miss or no cache - load from disk
  auto route = router.resolve(path);
  if (!route.found) {
    res.status = 404;
    res.body.assign({'N', 'o', 't', ' ', 'f', 'o', 'u', 'n', 'd'});
    return res;
  }

  std::filesystem::path full = route.file;

  std::error_code ec;
  auto size = std::filesystem::file_size(full, ec);
  if (ec) {
    res.status = 404;
    res.body.assign({'N', 'o', 't', ' ', 'f', 'o', 'u', 'n', 'd'});
    return res;
  }

  res.body.resize(static_cast<std::size_t>(size));
  std::ifstream in(full, std::ios::binary);
  if (!in.read(reinterpret_cast<char*>(res.body.data()),
               static_cast<std::streamsize>(res.body.size()))) {
    res.status = 500;
    res.body.assign({'E', 'r', 'r', 'o', 'r'});
    return res;
  }

  res.status = 200;
  res.content_type = guess_content_type(full);
  return res;
}

StaticResult load_static_vhost(std::string_view path,
                               const std::filesystem::path& document_root,
                               const Router& router) {
  StaticResult res;

  // Resolve using custom document root
  auto route = router.resolve(path, document_root);
  if (!route.found) {
    res.status = 404;
    res.body.assign({'N', 'o', 't', ' ', 'f', 'o', 'u', 'n', 'd'});
    return res;
  }

  std::filesystem::path full = route.file;

  std::error_code ec;
  auto size = std::filesystem::file_size(full, ec);
  if (ec) {
    res.status = 404;
    res.body.assign({'N', 'o', 't', ' ', 'f', 'o', 'u', 'n', 'd'});
    return res;
  }

  res.body.resize(static_cast<std::size_t>(size));
  std::ifstream in(full, std::ios::binary);
  if (!in.read(reinterpret_cast<char*>(res.body.data()),
               static_cast<std::streamsize>(res.body.size()))) {
    res.status = 500;
    res.body.assign({'E', 'r', 'r', 'o', 'r'});
    return res;
  }

  res.status = 200;
  res.content_type = guess_content_type(full);
  return res;
}

} // namespace neonsignal
