#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace neonsignal {

struct Http2Connection;

/**
 * Optimized SSE (Server-Sent Events) broadcaster
 * Batches events and pre-encodes HTTP/2 DATA frames for efficiency
 */
class SSEBroadcaster {
public:
  enum class Channel {
    Events,       // General application events
    CPUMetrics,   // CPU usage stats
    MemMetrics,   // Memory usage stats
    Redirect      // Redirect service status
  };

  // Batch interval - accumulate events for this duration
  static constexpr std::chrono::milliseconds BATCH_INTERVAL{16};  // ~60fps

  SSEBroadcaster() = default;

  // Subscribe connection to a channel
  void subscribe(Channel channel, int fd, std::shared_ptr<Http2Connection> conn,
                 std::uint32_t stream_id) {
    std::lock_guard lock(mutex_);
    Subscription sub{fd, std::move(conn), stream_id};
    subscriptions_[channel].insert(sub);
  }

  // Unsubscribe connection from channel
  void unsubscribe(Channel channel, int fd) {
    std::lock_guard lock(mutex_);
    auto& subs = subscriptions_[channel];
    std::erase_if(subs, [fd](const Subscription& sub) {
      return sub.fd == fd;
    });
  }

  // Unsubscribe from all channels
  void unsubscribe_all(int fd) {
    std::lock_guard lock(mutex_);
    for (auto& [_, subs] : subscriptions_) {
      std::erase_if(subs, [fd](const Subscription& sub) {
        return sub.fd == fd;
      });
    }
  }

  // Queue an event for broadcast (batched)
  void queue_event(Channel channel, std::string_view event_data) {
    std::lock_guard lock(mutex_);
    pending_events_[channel].emplace_back(event_data);
  }

  // Immediate broadcast (bypasses batching)
  void broadcast_immediate(Channel channel, std::string_view event_data,
                          std::function<void(const std::shared_ptr<Http2Connection>&,
                                             std::uint32_t, const std::vector<std::uint8_t>&)> send_frame) {
    auto frame = encode_sse_data_frame_(event_data);

    std::lock_guard lock(mutex_);
    auto it = subscriptions_.find(channel);
    if (it == subscriptions_.end()) return;

    for (const auto& sub : it->second) {
      if (auto conn = sub.conn.lock()) {
        send_frame(conn, sub.stream_id, frame);
      }
    }
  }

  // Flush batched events (call periodically, e.g., every 16ms)
  void flush_batched(std::function<void(const std::shared_ptr<Http2Connection>&,
                                        std::uint32_t, const std::vector<std::uint8_t>&)> send_frame) {
    std::unordered_map<Channel, std::vector<std::string>> events_snapshot;

    {
      std::lock_guard lock(mutex_);
      events_snapshot = std::move(pending_events_);
      pending_events_.clear();
    }

    // Process each channel
    for (const auto& [channel, events] : events_snapshot) {
      if (events.empty()) continue;

      // Combine multiple events into a single frame (if small enough)
      std::string combined;
      for (const auto& event : events) {
        combined += event;
        combined += "\n\n";  // SSE event separator
      }

      // Pre-encode once for all subscribers
      auto frame = encode_sse_data_frame_(combined);

      // Broadcast to all subscribers of this channel
      std::lock_guard lock(mutex_);
      auto it = subscriptions_.find(channel);
      if (it == subscriptions_.end()) continue;

      for (const auto& sub : it->second) {
        if (auto conn = sub.conn.lock()) {
          send_frame(conn, sub.stream_id, frame);
        }
      }
    }
  }

  // Get subscriber count for a channel
  [[nodiscard]] std::size_t subscriber_count(Channel channel) const {
    std::lock_guard lock(mutex_);
    auto it = subscriptions_.find(channel);
    return (it != subscriptions_.end()) ? it->second.size() : 0;
  }

  // Get total subscriber count
  [[nodiscard]] std::size_t total_subscribers() const {
    std::lock_guard lock(mutex_);
    std::size_t total = 0;
    for (const auto& [_, subs] : subscriptions_) {
      total += subs.size();
    }
    return total;
  }

  // Simple iteration over subscribers (for custom frame building)
  template<typename Callback>
  void for_each_subscriber(Channel channel, Callback&& callback) {
    std::lock_guard lock(mutex_);
    auto it = subscriptions_.find(channel);
    if (it == subscriptions_.end()) return;

    // Iterate and call callback for each active subscriber
    for (const auto& sub : it->second) {
      if (auto conn = sub.conn.lock()) {
        callback(conn, sub.stream_id);
      }
    }
  }

private:
  struct Subscription {
    int fd;
    std::weak_ptr<Http2Connection> conn;
    std::uint32_t stream_id;

    bool operator==(const Subscription& other) const {
      return fd == other.fd && stream_id == other.stream_id;
    }
  };

  struct SubscriptionHash {
    std::size_t operator()(const Subscription& sub) const {
      return std::hash<int>{}(sub.fd) ^ (std::hash<std::uint32_t>{}(sub.stream_id) << 1);
    }
  };

  // Encode SSE data as HTTP/2 DATA frame
  [[nodiscard]] std::vector<std::uint8_t> encode_sse_data_frame_(std::string_view data) const {
    // HTTP/2 DATA frame format:
    // +-----------------------------------------------+
    // |                 Length (24)                   |
    // +---------------+---------------+---------------+
    // |   Type (8)    |   Flags (8)   |
    // +-+-------------+---------------+-------------------------------+
    // |R|                 Stream Identifier (31)                      |
    // +=+=============================================================+
    // |                   Data (*)                   ...
    // +---------------------------------------------------------------+

    std::vector<std::uint8_t> frame;
    frame.reserve(9 + data.size());

    // Length (24 bits)
    std::uint32_t length = static_cast<std::uint32_t>(data.size());
    frame.push_back((length >> 16) & 0xFF);
    frame.push_back((length >> 8) & 0xFF);
    frame.push_back(length & 0xFF);

    // Type: DATA (0x00)
    frame.push_back(0x00);

    // Flags: None (stream stays open)
    frame.push_back(0x00);

    // Stream ID will be filled by caller (4 bytes placeholder)
    frame.push_back(0x00);
    frame.push_back(0x00);
    frame.push_back(0x00);
    frame.push_back(0x00);

    // Data payload
    frame.insert(frame.end(), data.begin(), data.end());

    return frame;
  }

  mutable std::mutex mutex_;
  std::unordered_map<Channel, std::unordered_set<Subscription, SubscriptionHash>> subscriptions_;
  std::unordered_map<Channel, std::vector<std::string>> pending_events_;
};

} // namespace neonsignal
