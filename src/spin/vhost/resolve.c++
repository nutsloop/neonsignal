#include "spin/vhost.h++"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <mutex>

namespace neonsignal {
NeonJSXConfig parse_neonjsx_config(const std::filesystem::path &config_path) {
  std::string domain_name;
  if (const auto parent = config_path.parent_path(); !parent.empty()) {
    domain_name = parent.filename().string();
  }
  NeonJSXConfig config;
  std::ifstream file(config_path);
  if (!file.is_open()) {
    if (!domain_name.empty()) {
      std::cerr << "• vhost: " << domain_name << " [neonjsx: disabled]\n";
    }
    return config;
  }

  config.enabled = true;
  std::string line;
  while (std::getline(file, line)) {
    const auto start = line.find_first_not_of(" \t");
    if (start == std::string::npos) {
      continue;
    }
    const auto end = line.find_last_not_of(" \t\r\n");
    line = line.substr(start, end - start + 1);
    if (line.empty() || line[0] == '#') {
      continue;
    }
    if (line.size() > 2 && line.ends_with("/*")) {
      config.wildcard_routes.push_back(line.substr(0, line.size() - 2));
    } else {
      config.routes.push_back(line);
    }
  }

  if (!domain_name.empty()) {
    std::size_t count = config.routes.size() + config.wildcard_routes.size();
    std::cerr << "• vhost: " << domain_name << " [neonjsx: " << count << " routes]\n";
  }
  return config;
}

bool NeonJSXConfig::matches_route(const std::string_view path) const {
  if (!enabled) {
    return false;
  }
  for (const auto &route : routes) {
    if (path == route) {
      return true;
    }
  }
  for (const auto &prefix : wildcard_routes) {
    if (path.starts_with(prefix)) {
      return true;
    }
  }
  return false;
}

std::string VHostResolver::normalize_authority(std::string_view authority) {
  // Strip port if present: "example.com:443" -> "example.com"
  if (const auto pos = authority.find(':'); pos != std::string_view::npos) {
    authority = authority.substr(0, pos);
  }
  // Convert to lowercase for case-insensitive matching
  std::string result(authority);
  std::ranges::transform(result, result.begin(),
                         [](const unsigned char c) { return std::tolower(c); });
  return result;
}

bool VHostResolver::is_domain_directory(const std::string_view name) {
  // Match: domain.tld, sub.domain.tld, _default
  if (name == "_default") return true;
  if (name.empty() || name[0] == '.' || name.back() == '.') return false;

  // Must contain at least one dot (domain.tld format)
  bool has_dot = false;
  bool prev_was_dot = false;

  for (const char c : name) {
    if (c == '.') {
      if (prev_was_dot) return false; // No consecutive dots
      has_dot = true;
      prev_was_dot = true;
    } else if (c == '-' || std::isalnum(static_cast<unsigned char>(c))) {
      prev_was_dot = false;
    } else {
      return false; // Invalid character
    }
  }

  return has_dot;
}

void VHostResolver::refresh() {
  std::unique_lock lock(mutex_);
  exact_hosts_.clear();
  wildcard_hosts_.clear();
  has_default_ = false;

  if (!std::filesystem::is_directory(public_root_)) {
    return;
  }

  for (const auto &entry : std::filesystem::directory_iterator(public_root_)) {
    if (!entry.is_directory()) continue;

    std::string name = entry.path().filename().string();
    if (!is_domain_directory(name)) continue;

    VirtualHost vhost;
    vhost.domain = name;
    vhost.document_root = entry.path();
    vhost.neonjsx = parse_neonjsx_config(entry.path() / ".neonjsx");

    if (name == "_default") {
      has_default_ = true;
      exact_hosts_["_default"] = std::move(vhost);
    } else {
      // Normalize domain name to lowercase
      std::string normalized = name;
      std::ranges::transform(normalized, normalized.begin(),
                             [](const unsigned char c) { return std::tolower(c); });
      exact_hosts_[normalized] = std::move(vhost);
    }
  }
}

std::optional<std::filesystem::path>
VHostResolver::resolve(const std::string_view authority) const {
  std::shared_lock lock(mutex_);

  if (exact_hosts_.empty()) {
    return std::nullopt; // No vhosting configured
  }

  const std::string domain = normalize_authority(authority);

  // 1. Exact match
  if (const auto it = exact_hosts_.find(domain); it != exact_hosts_.end()) {
    return it->second.document_root;
  }

  // 2. Default fallback
  if (has_default_) {
    return exact_hosts_.at("_default").document_root;
  }

  return std::nullopt;
}

bool VHostResolver::enabled() const {
  std::shared_lock lock(mutex_);
  // Enabled if we have at least one non-default vhost
  return std::ranges::any_of(exact_hosts_,
                             [](const auto &entry) { return entry.first != "_default"; });
}

bool VHostResolver::is_neonjsx(const std::string_view authority) const {
  std::shared_lock lock(mutex_);
  const std::string domain = normalize_authority(authority);

  if (const auto it = exact_hosts_.find(domain); it != exact_hosts_.end()) {
    return it->second.neonjsx.enabled;
  }
  if (has_default_) {
    return exact_hosts_.at("_default").neonjsx.enabled;
  }
  return false;
}

bool VHostResolver::is_neonjsx_route(const std::string_view authority,
                                     const std::string_view path) const {
  std::shared_lock lock(mutex_);
  const std::string domain = normalize_authority(authority);

  if (const auto it = exact_hosts_.find(domain); it != exact_hosts_.end()) {
    return it->second.neonjsx.matches_route(path);
  }
  if (has_default_) {
    return exact_hosts_.at("_default").neonjsx.matches_route(path);
  }
  return false;
}

std::vector<std::string> VHostResolver::list_vhosts() const {
  std::shared_lock lock(mutex_);
  std::vector<std::string> result;
  result.reserve(exact_hosts_.size());
  for (const auto &[name, vhost] : exact_hosts_) {
    result.push_back(name + " -> " + vhost.document_root.string());
  }
  std::ranges::sort(result);
  return result;
}
} // namespace neonsignal
