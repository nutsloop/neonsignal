#include "spin/hpack_decoder.h++"

#include <optional>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace neonsignal
{
  std::optional<ParsedHeaders> HpackDecoder::decode(const std::vector<std::uint8_t>& block) const
  {
    if (inflater_ == nullptr)
    {
      return std::nullopt;
    }

    ParsedHeaders out;
    std::size_t off = 0;

    while (off < block.size())
    {
      nghttp2_nv nv{};
      int inflate_flags = 0;

      ssize_t rv = nghttp2_hd_inflate_hd2(inflater_, &nv, &inflate_flags,
                                          block.data() + off,
                                          block.size() - off, 1);
      if (rv < 0)
      {
        return std::nullopt;
      }
      off += static_cast<std::size_t>(rv);

      if (inflate_flags & NGHTTP2_HD_INFLATE_EMIT)
      {
        std::string name(reinterpret_cast<char const*>(nv.name), nv.namelen);
        std::string value(reinterpret_cast<char const*>(nv.value), nv.valuelen);
        std::ranges::transform(name, name.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (name == ":path")
        {
          out.path = value;
        }
        else if (name == ":method")
        {
          out.method = value;
        }
        else if (name == ":authority")
        {
          out.authority = value;
        }
        else if (name == ":scheme")
        {
          out.scheme = value;
        }
        else
        {
          if (name == "cookie")
          {
            auto it = out.headers.find("cookie");
            if (it != out.headers.end())
            {
              it->second.append("; ");
              it->second.append(value);
              continue;
            }
          }
          out.headers[name] = value;
        }
      }

      if (inflate_flags & NGHTTP2_HD_INFLATE_FINAL)
      {
        break;
      }
    }

    nghttp2_hd_inflate_end_headers(inflater_);

    if (out.path.empty())
    {
      return std::nullopt;
    }
    return out;
  }
} // namespace neonsignal
