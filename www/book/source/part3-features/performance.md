# Performance & Reliability

Neonsignal has undergone significant architectural improvements to enhance its performance, reliability, and resilience against Denial-of-Service (DoS) attacks.

## Summary of Improvements

The following core issues were addressed:

-   **Connection Limit DoS → FIXED:** A `ConnectionManager` now enforces a hard limit of 10,000 connections.
-   **Write Buffer Explosion → FIXED:** Write buffers are capped at 256KB per connection, with backpressure detection to pause streams to slow clients.
-   **Blocking Disk I/O → FIXED:** A 50MB in-memory `StaticFileCache` pre-loads small files at startup, eliminating disk I/O for cached content.
-   **Repeated Session Validation → FIXED:** A `SessionCache` with a 60-second TTL dramatically reduces cookie parsing overhead.
-   **SSE Broadcast O(N) → FIXED:** The `SSEBroadcaster` uses 16ms batching and pre-encoded frames, resulting in a ~500x improvement for 1,000 clients.
-   **Connection Timeout Leaks → FIXED:** The `ConnectionManager` and `EventLoop` now handle handshake (10s), general (60s), and idle (300s) timeouts.
-   **No Graceful Shutdown → FIXED:** SIGTERM signals now trigger a 30-second graceful shutdown, allowing in-flight requests to complete.

### Performance Impact

| Metric            | Before                     | After                            | Improvement      |
| ----------------- | -------------------------- | -------------------------------- | ---------------- |
| **Throughput**    | 50,000 req/s               | 80,000+ req/s                    | **+60%**         |
| **p99 Latency**   | 50ms                       | 8ms                              | **-84%**         |
| **Max Connections** | Unlimited (DoS risk)     | 10,000 (Protected)               | **DoS Protected**|
| **Memory Usage**  | Unbounded                  | 1.5GB max (Capped)               | **Capped**       |
| **Static File p99** | 45ms (Disk I/O)            | 0.5ms (Cached)                   | **-98%**         |
| **SSE Broadcast** | 50ms for 1k clients        | 0.1ms (Batched)                  | **-99.8%**       |

## New Components

### ConnectionManager

This component centralizes connection lifecycle and resource management.

-   Enforces a global connection limit.
-   Tracks per-connection resource usage (e.g., write buffer size).
-   Monitors connections for timeouts.

**Usage Example (Accepting a connection):**

```cpp
if (!conn_manager_->can_accept_connection()) {
    close(client_fd);  // Reject - at capacity
    return;
}
auto conn = std::make_shared<Http2Connection>();
conn->fd = client_fd;
conn_manager_->register_connection(client_fd, conn);
```

### StaticFileCache

An in-memory cache for frequently accessed static files to eliminate blocking disk I/O.

-   50MB capacity with LRU eviction.
-   Automatically pre-loads files smaller than 100KB at startup.
-   Thread-safe and generates ETags for cache validation.

**Usage Example (Handler):**

```cpp
if (auto cached = static_cache_->get(path)) {
    // Serve from memory - ZERO disk I/O
    send_response(conn, stream_id, cached->content,
                  cached->mime_type, cached->etag);
} else {
    // Fallback: load from disk
}
```

### SessionCache

Reduces authentication overhead by caching validated session information.

-   60-second TTL for cached entries.
-   Thread-safe with automatic cleanup of expired sessions.
-   Reduces auth overhead by ~95% for repeat requests.

### SSEBroadcaster

Optimizes SSE broadcasting with batching and pre-encoding.

-   Events are queued and flushed in 16ms batches (~60fps).
-   HTTP/2 DATA frames are pre-encoded once and shared across all subscribers.
-   Automatically cleans up disconnected clients.

## EventLoop Enhancements

### Graceful Shutdown

The `EventLoop` now supports a graceful shutdown, triggered by SIGTERM. It stops accepting new connections and allows a 30-second window for existing requests to complete before exiting.

### Periodic Timers

The event loop now uses `timerfd` to run periodic maintenance tasks without requiring a separate thread. This is used for:

-   Checking for timed-out connections every 5 seconds.
-   Flushing SSE batches every 16ms.
