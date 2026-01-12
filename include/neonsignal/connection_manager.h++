#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace neonsignal {

// Http2Connection is defined in http2_listener.h++, which includes this header
// No forward declaration needed as it's already defined before this include

/**
 * Connection resource management and limits
 * Prevents DoS attacks and resource exhaustion
 */
class ConnectionManager {
public:
  // Resource limits - tuned for production
  static constexpr std::size_t MAX_CONNECTIONS = 10'000;
  static constexpr std::size_t MAX_STREAMS_PER_CONNECTION = 100;
  static constexpr std::size_t MAX_WRITE_BUFFER_BYTES = 256 * 1024;       // 256KB
  static constexpr std::size_t MAX_READ_BUFFER_BYTES = 1024 * 1024;       // 1MB
  static constexpr std::size_t MAX_UPLOAD_SIZE_BYTES = 100 * 1024 * 1024; // 100MB
  static constexpr std::chrono::seconds CONNECTION_TIMEOUT{60};
  static constexpr std::chrono::seconds HANDSHAKE_TIMEOUT{10};
  static constexpr std::chrono::seconds IDLE_TIMEOUT{300}; // 5 minutes

  ConnectionManager() = default;

  // Check if we can accept a new connection
  [[nodiscard]] bool can_accept_connection() const {
    return connection_count_.load(std::memory_order_relaxed) < MAX_CONNECTIONS;
  }

  // Register a new connection
  void register_connection(int fd, std::shared_ptr<Http2Connection> conn) {
    std::lock_guard lock(mutex_);
    connections_[fd] = std::move(conn);
    connection_count_.fetch_add(1, std::memory_order_relaxed);
  }

  // Get connection by fd
  [[nodiscard]] std::shared_ptr<Http2Connection> get_connection(int fd) {
    std::lock_guard lock(mutex_);
    auto it = connections_.find(fd);
    return (it != connections_.end()) ? it->second : nullptr;
  }

  // Remove connection
  void remove_connection(int fd) {
    std::lock_guard lock(mutex_);
    if (connections_.erase(fd)) {
      connection_count_.fetch_sub(1, std::memory_order_relaxed);
    }
  }

  // Get all connections (for broadcast/timeout checks)
  [[nodiscard]] std::vector<std::shared_ptr<Http2Connection>> get_all_connections() {
    std::lock_guard lock(mutex_);
    std::vector<std::shared_ptr<Http2Connection>> result;
    result.reserve(connections_.size());
    for (const auto &[_, conn] : connections_) {
      result.push_back(conn);
    }
    return result;
  }

  // Check for timed out connections
  [[nodiscard]] std::vector<int> find_timed_out_connections() {
    std::vector<int> timed_out;
    auto now = std::chrono::steady_clock::now();

    std::lock_guard lock(mutex_);
    for (const auto &[fd, conn] : connections_) {
      auto idle_time = now - conn->last_activity;

      // Check handshake timeout
      if (!conn->handshake_complete && now > conn->handshake_deadline) {
        timed_out.push_back(fd);
        continue;
      }

      // Check idle timeout
      if (idle_time > IDLE_TIMEOUT) {
        timed_out.push_back(fd);
      }
    }

    return timed_out;
  }

  // Statistics
  [[nodiscard]] std::size_t connection_count() const {
    return connection_count_.load(std::memory_order_relaxed);
  }

  [[nodiscard]] std::size_t total_write_buffer_bytes() const {
    std::size_t total = 0;
    std::lock_guard lock(mutex_);
    for (const auto &[_, conn] : connections_) {
      total += conn->write_buf.size();
    }
    return total;
  }

  // Check write buffer backpressure
  [[nodiscard]] bool has_write_backpressure(const std::shared_ptr<Http2Connection> &conn) const {
    return conn->write_buf.size() > MAX_WRITE_BUFFER_BYTES;
  }

  // Clear all connections (for shutdown)
  [[nodiscard]] std::vector<int> drain_all_connections() {
    std::lock_guard lock(mutex_);
    std::vector<int> fds;
    fds.reserve(connections_.size());
    for (const auto &[fd, _] : connections_) {
      fds.push_back(fd);
    }
    return fds;
  }

  // Iterate through all connections with callback
  template <typename Func> void for_each_connection(Func callback) {
    std::lock_guard lock(mutex_);
    for (auto &[fd, conn] : connections_) {
      callback(fd, conn);
    }
  }

private:
  mutable std::mutex mutex_;
  std::unordered_map<int, std::shared_ptr<Http2Connection>> connections_;
  std::atomic<std::size_t> connection_count_{0};
};

} // namespace neonsignal
