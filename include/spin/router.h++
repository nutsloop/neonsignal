#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace neonsignal {

struct RouteResult {
  std::filesystem::path file;
  bool found{false};
};

class Router {
public:
  explicit Router(std::filesystem::path public_root);

  [[nodiscard]] RouteResult resolve(std::string_view path) const;

  // Resolve with custom document root (for virtual hosting)
  [[nodiscard]] RouteResult resolve(std::string_view path,
                                    const std::filesystem::path &document_root) const;

private:
  std::filesystem::path public_root_;
};

} // namespace neonsignal
