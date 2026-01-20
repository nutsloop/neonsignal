#pragma once

#include <cstdint>

namespace neonsignal {

// OS-neutral event flags for EventLoop
// These abstract over epoll (Linux) and kqueue (macOS/BSD)
namespace EventMask {

// Readable - data available to read
inline constexpr std::uint32_t Read = 1U << 0;

// Writable - ready to write without blocking
inline constexpr std::uint32_t Write = 1U << 1;

// Error condition on the file descriptor
inline constexpr std::uint32_t Error = 1U << 2;

// Hang up - peer closed connection
inline constexpr std::uint32_t HangUp = 1U << 3;

// Edge-triggered mode (only notify on state changes)
// Linux: EPOLLET
// macOS: EV_CLEAR
inline constexpr std::uint32_t Edge = 1U << 4;

// Peer closed write side (read will return 0)
inline constexpr std::uint32_t ReadHangUp = 1U << 5;

// Common combinations
inline constexpr std::uint32_t ReadEdge = Read | Edge;
inline constexpr std::uint32_t ReadWrite = Read | Write;
inline constexpr std::uint32_t ReadWriteEdge = Read | Write | Edge;

} // namespace EventMask

} // namespace neonsignal
