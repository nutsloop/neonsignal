#pragma once

#include <filesystem>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace neonsignal {

struct NeonJSXConfig {
  bool enabled{false};
  std::vector<std::string> routes;
  std::vector<std::string> wildcard_routes;

  [[nodiscard]] bool matches_route(std::string_view path) const;
};

struct VirtualHost {
  std::string domain;
  std::filesystem::path document_root;
  bool is_wildcard{false};
  NeonJSXConfig neonjsx;
};

class VHostResolver {
public:
  explicit VHostResolver(std::filesystem::path public_root);

  // Resolve domain to document root
  // Returns nullopt if no vhost found (use legacy behavior)
  [[nodiscard]] std::optional<std::filesystem::path> resolve(std::string_view authority) const;

  // Rescan public directory for vhost directories
  void refresh();

  // Check if vhosting is enabled (any vhost dirs exist)
  [[nodiscard]] bool enabled() const;

  // Check if domain has NeonJSX SPA routing enabled
  [[nodiscard]] bool is_neonjsx(std::string_view authority) const;

  // Check if path is a known NeonJSX route for this domain
  [[nodiscard]] bool is_neonjsx_route(std::string_view authority, std::string_view path) const;

  // Get list of discovered vhosts for logging
  [[nodiscard]] std::vector<std::string> list_vhosts() const;

private:
  std::filesystem::path public_root_;
  mutable std::shared_mutex mutex_;
  std::unordered_map<std::string, VirtualHost> exact_hosts_;
  std::vector<VirtualHost> wildcard_hosts_;
  bool has_default_{false};

  // Extract domain from authority (strips port if present)
  static std::string normalize_authority(std::string_view authority);

  // Check if directory name looks like a domain
  static bool is_domain_directory(std::string_view name);
};

} // namespace neonsignal
