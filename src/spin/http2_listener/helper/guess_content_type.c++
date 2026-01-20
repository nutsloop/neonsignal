#include "spin/http2_listener_helpers.h++"

#include <unordered_map>

namespace neonsignal {

std::string guess_content_type(const std::filesystem::path& p) {
  static const std::unordered_map<std::string, std::string> kMimeTable = {
      {".html", "text/html; charset=utf-8"},     {".htm", "text/html; charset=utf-8"},
      {".css", "text/css; charset=utf-8"},       {".js", "application/javascript"},
      {".mjs", "application/javascript"},        {".json", "application/json"},
      {".txt", "text/plain; charset=utf-8"},     {".png", "image/png"},
      {".jpg", "image/jpeg"},                    {".jpeg", "image/jpeg"},
      {".gif", "image/gif"},                     {".svg", "image/svg+xml"},
      {".ico", "image/x-icon"},                  {".webp", "image/webp"},
      {".avif", "image/avif"},                   {".mp4", "video/mp4"},
      {".webm", "video/webm"},                   {".ogg", "audio/ogg"},
      {".mp3", "audio/mpeg"},                    {".wav", "audio/wav"},
      {".wasm", "application/wasm"},             {".xml", "application/xml"},
      {".pdf", "application/pdf"},               {".zip", "application/zip"},
  };

  auto ext = p.extension().string();
  auto it = kMimeTable.find(ext);
  if (it != kMimeTable.end()) {
    return it->second;
  }
  return "application/octet-stream";
}

} // namespace neonsignal
