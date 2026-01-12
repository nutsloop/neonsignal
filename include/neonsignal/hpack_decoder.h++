#pragma once

#include <nghttp2/nghttp2.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace neonsignal {

struct ParsedHeaders {
  std::string method;
  std::string path;
  std::string authority;
  std::string scheme;
  std::unordered_map<std::string, std::string> headers;
};

class HpackDecoder {
public:
  HpackDecoder();
  ~HpackDecoder();

  // Returns std::nullopt on decode failure.
  std::optional<ParsedHeaders> decode(const std::vector<std::uint8_t> &block) const;

private:
  nghttp2_hd_inflater *inflater_{nullptr};
};

} // namespace neonsignal
