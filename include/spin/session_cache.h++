#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace neonsignal {

/**
 * Session validation cache
 * Avoid re-parsing and re-validating cookies on every request
 */
class SessionCache {
public:
  struct SessionInfo {
    std::string user_id;
    std::string credential_id;
    std::chrono::steady_clock::time_point cached_at;
    std::chrono::steady_clock::time_point expires_at;
    bool authenticated{false};
  };

  // Cache TTL - revalidate after this period
  static constexpr std::chrono::seconds CACHE_TTL{60};

  SessionCache() = default;

  // Get cached session info
  [[nodiscard]] std::optional<SessionInfo> get(std::string_view session_token) const {
    std::lock_guard lock(mutex_);
    auto it = cache_.find(std::string(session_token));

    if (it == cache_.end()) {
      return std::nullopt;
    }

    auto now = std::chrono::steady_clock::now();

    // Check if expired
    if (now > it->second.expires_at) {
      return std::nullopt;
    }

    // Check if needs refresh
    if (now - it->second.cached_at > CACHE_TTL) {
      return std::nullopt;  // Force revalidation
    }

    return it->second;
  }

  // Store session info
  void put(std::string_view session_token, SessionInfo info) {
    std::lock_guard lock(mutex_);
    info.cached_at = std::chrono::steady_clock::now();
    cache_[std::string(session_token)] = std::move(info);
  }

  // Invalidate session
  void invalidate(std::string_view session_token) {
    std::lock_guard lock(mutex_);
    cache_.erase(std::string(session_token));
  }

  // Clear all sessions
  void clear() {
    std::lock_guard lock(mutex_);
    cache_.clear();
  }

  // Cleanup expired entries (call periodically)
  void cleanup() {
    std::lock_guard lock(mutex_);
    auto now = std::chrono::steady_clock::now();

    for (auto it = cache_.begin(); it != cache_.end();) {
      if (now > it->second.expires_at) {
        it = cache_.erase(it);
      } else {
        ++it;
      }
    }
  }

  // Statistics
  [[nodiscard]] std::size_t size() const {
    std::lock_guard lock(mutex_);
    return cache_.size();
  }

private:
  mutable std::mutex mutex_;
  std::unordered_map<std::string, SessionInfo> cache_;
};

} // namespace neonsignal
