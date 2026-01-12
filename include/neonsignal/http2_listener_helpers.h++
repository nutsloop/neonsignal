#pragma once

#include "neonsignal/neonsignal.h++"
#include "neonsignal/router.h++"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace neonsignal {

inline constexpr std::string_view kClientPreface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
inline constexpr std::size_t kMaxHeaderLog = 256;

struct StaticResult {
  int status{500};
  std::string content_type{"text/plain; charset=utf-8"};
  std::vector<std::uint8_t> body;
};

/**
 * Create a non-blocking TCP listen socket bound to the configured host/port.
 *
 * @param config Server address/port options.
 * @return Listening socket fd (caller responsible for close), or throws on
 *         failure.
 */
int make_listen_socket(const ServerConfig &config);
/**
 * Read the current process RSS in kilobytes from /proc/self/status.
 *
 * @return RSS in kilobytes, or 0 if it cannot be determined.
 */
std::uint64_t read_rss_kb();

/**
 * HPACK integer encoding helper (RFC 7541 §5.1).
 *
 * @param out Destination buffer to append to.
 * @param value Integer to encode.
 * @param prefix_bits Number of prefix bits allowed in the first byte (1-8).
 * @param first_byte_prefix High bits to OR into the first byte.
 */
void encode_integer(std::vector<std::uint8_t> &out, std::uint32_t value, std::uint8_t prefix_bits,
                    std::uint8_t first_byte_prefix);
/**
 * HPACK string encoding helper (no Huffman, H=0).
 *
 * @param out Destination buffer to append to.
 * @param str UTF-8 string to encode with length prefix.
 */
void encode_string(std::vector<std::uint8_t> &out, std::string_view str);
/**
 * Emit a literal header field without indexing.
 *
 * @param out Destination buffer to append to.
 * @param name_index HPACK static table index for the header name.
 * @param value Header value to encode (no Huffman).
 */
void encode_literal_header_no_index(std::vector<std::uint8_t> &out, std::uint32_t name_index,
                                    std::string_view value);
/**
 * @brief Build a raw HTTP/2 frame.
 *
 * Constructs the 9-byte HTTP/2 frame header (length, type, flags, stream id)
 * followed by the supplied payload. The caller is responsible for ensuring the
 * payload is valid for the given frame type and flags.
 *
 * @param type Frame type byte (e.g., 0x0 DATA, 0x1 HEADERS).
 * @param flags Frame flags bitmask (END_STREAM, END_HEADERS, etc.).
 * @param stream_id Stream identifier (highest bit must remain 0 per spec).
 * @param payload Frame payload to append after the header.
 * @return std::vector<std::uint8_t> Complete frame ready to send on the wire.
 */
std::vector<std::uint8_t> build_frame(std::uint8_t type, std::uint8_t flags,
                                      std::uint32_t stream_id,
                                      const std::vector<std::uint8_t> &payload);
/**
 * HPACK integer decoding helper.
 *
 * @param buf Source buffer.
 * @param off In/out offset; advanced past the integer on success.
 * @param prefix_bits Number of prefix bits encoded in the first byte.
 * @param out_val Decoded integer on success.
 * @return true on success, false if buffer is too short or malformed.
 */
bool decode_integer(const std::vector<std::uint8_t> &buf, std::size_t &off,
                    std::uint8_t prefix_bits, std::uint32_t &out_val);
/**
 * Guess MIME type from file extension.
 *
 * @param p Path to inspect.
 * @return MIME string (defaults to application/octet-stream).
 */
std::string guess_content_type(const std::filesystem::path &p);
/**
 * Build server SETTINGS frame payload for startup (max streams + window).
 *
 * @return Encoded SETTINGS frame.
 */
std::vector<std::uint8_t> build_server_settings();
/**
 * Build SETTINGS ACK frame.
 *
 * @return Encoded SETTINGS ACK frame.
 */
std::vector<std::uint8_t> build_settings_ack();
/**
 * Encode HEADERS/DATA frames for a full HTTP/2 response.
 *
 * @param out Buffer to append frames to.
 * @param stream_id Target stream id.
 * @param status HTTP status code.
 * @param content_type Content-Type header value.
 * @param body Response body payload.
 */
void build_response_frames(std::vector<std::uint8_t> &out, std::uint32_t stream_id, int status,
                           std::string_view content_type, const std::vector<std::uint8_t> &body);
/**
 * Encode HEADERS/DATA frames for a response with extra headers (e.g., cookies).
 *
 * @param out Buffer to append frames to.
 * @param stream_id Target stream id.
 * @param status HTTP status code.
 * @param content_type Content-Type header value.
 * @param extra_headers Additional header key/value pairs to emit.
 * @param body Response body payload.
 */
void build_response_frames_with_headers(
    std::vector<std::uint8_t> &out, std::uint32_t stream_id, int status,
    std::string_view content_type,
    const std::vector<std::pair<std::string, std::string>> &extra_headers,
    const std::vector<std::uint8_t> &body);
/**
 * Build a WINDOW_UPDATE frame.
 *
 * @param stream_id Stream id (0 for connection-level).
 * @param increment Window increment value.
 * @return Encoded WINDOW_UPDATE frame.
 */
std::vector<std::uint8_t> build_window_update(std::uint32_t stream_id, std::uint32_t increment);
// Forward declaration for cache
class StaticFileCache;

/**
 * Resolve and load a static asset from the router/public root.
 * Checks in-memory cache first, falls back to disk on cache miss.
 *
 * @param path HTTP path (leading slash).
 * @param router Router abstraction that maps paths to files.
 * @param cache Optional cache for in-memory file serving (nullptr = no cache).
 * @return StaticResult with status/content-type/body.
 */
StaticResult load_static(std::string_view path, const Router &router,
                         StaticFileCache *cache = nullptr);

/**
 * Resolve and load a static asset from a custom document root (for vhosting).
 * Does NOT use cache (vhost paths vary by domain).
 *
 * @param path HTTP path (leading slash).
 * @param document_root Custom root directory for this virtual host.
 * @param router Router abstraction (uses its resolve(path, doc_root) overload).
 * @return StaticResult with status/content-type/body.
 */
StaticResult load_static_vhost(std::string_view path, const std::filesystem::path &document_root,
                               const Router &router);

} // namespace neonsignal
