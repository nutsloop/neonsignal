#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace neonsignal {

/**
 * In-memory cache for frequently accessed static files
 * Eliminates blocking disk I/O on the event loop thread
 */
class StaticFileCache {
public:
  struct CachedFile {
    std::vector<std::uint8_t> content;
    std::string mime_type;
    std::string etag; // For cache validation
    std::chrono::system_clock::time_point last_modified;
    std::size_t access_count{0};
  };

  explicit StaticFileCache(std::size_t max_cache_size_bytes = 50 * 1024 * 1024)
      : max_cache_size_bytes_(max_cache_size_bytes) {}

  // Pre-load critical files at startup
  void preload(const std::filesystem::path &public_root) {
    // Critical files that should ALWAYS be in memory
    const std::vector<std::string> critical_files = {"index.html", "app.js", "app.css",
                                                     "favicon.ico"};

    for (const auto &file : critical_files) {
      auto path = public_root / file;
      if (std::filesystem::exists(path)) {
        load_file(path, "/" + file);
      }
    }

    // Scan and pre-load small files (< 100KB)
    for (const auto &entry : std::filesystem::recursive_directory_iterator(public_root)) {
      if (entry.is_regular_file() && entry.file_size() < 100 * 1024) {
        auto rel_path = std::filesystem::relative(entry.path(), public_root);
        load_file(entry.path(), "/" + rel_path.string());
      }
    }

    std::cerr << std::format("📦 Static cache: {} files loaded, {} bytes\n", cache_.size(),
                             current_size_bytes_);
  }

  // Get file from cache
  [[nodiscard]] std::optional<CachedFile> get(std::string_view path) {
    std::lock_guard lock(mutex_);
    auto it = cache_.find(std::string(path));
    if (it != cache_.end()) {
      it->second.access_count++;
      return it->second;
    }
    return std::nullopt;
  }

  // Manually add file to cache
  void put(std::string_view path, std::vector<std::uint8_t> content, std::string_view mime_type) {
    std::lock_guard lock(mutex_);

    // Evict if over size limit
    while (current_size_bytes_ + content.size() > max_cache_size_bytes_ && !cache_.empty()) {
      evict_lru_();
    }

    CachedFile file;
    file.content = std::move(content);
    file.mime_type = mime_type;
    file.etag = generate_etag_(file.content);
    file.last_modified = std::chrono::system_clock::now();
    file.access_count = 0;

    current_size_bytes_ += file.content.size();
    cache_[std::string(path)] = std::move(file);
  }

  // Check if file is in cache
  [[nodiscard]] bool contains(std::string_view path) const {
    std::lock_guard lock(mutex_);
    return cache_.contains(std::string(path));
  }

  // Clear cache
  void clear() {
    std::lock_guard lock(mutex_);
    cache_.clear();
    current_size_bytes_ = 0;
  }

  // Statistics
  [[nodiscard]] std::size_t size() const {
    std::lock_guard lock(mutex_);
    return cache_.size();
  }

  [[nodiscard]] std::size_t bytes() const { return current_size_bytes_; }

private:
  void load_file(const std::filesystem::path &file_path, const std::string &cache_key) {
    try {
      std::ifstream file(file_path, std::ios::binary);
      if (!file) return;

      // Read entire file
      file.seekg(0, std::ios::end);
      auto size = file.tellg();
      file.seekg(0, std::ios::beg);

      std::vector<std::uint8_t> content(size);
      file.read(reinterpret_cast<char *>(content.data()), size);

      // Determine MIME type from extension
      auto mime = mime_type_from_extension_(file_path.extension().string());

      put(cache_key, std::move(content), mime);

    } catch (const std::exception &e) {
      std::cerr << std::format("Failed to load {}: {}\n", file_path.string(), e.what());
    }
  }

  void evict_lru_() {
    // Find least recently used (lowest access_count)
    auto lru = cache_.begin();
    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
      if (it->second.access_count < lru->second.access_count) {
        lru = it;
      }
    }

    if (lru != cache_.end()) {
      current_size_bytes_ -= lru->second.content.size();
      cache_.erase(lru);
    }
  }

  [[nodiscard]] std::string generate_etag_(const std::vector<std::uint8_t> &content) const {
    // Simple hash-based ETag (could use MD5/SHA for production)
    std::size_t hash = std::hash<std::string_view>{}(
        std::string_view(reinterpret_cast<const char *>(content.data()), content.size()));
    return std::format("\"{:016x}\"", hash);
  }

  [[nodiscard]] std::string mime_type_from_extension_(std::string_view ext) const {
    static const std::unordered_map<std::string_view, std::string_view> mime_types = {
        {".html", "text/html; charset=utf-8"},
        {".htm", "text/html; charset=utf-8"},
        {".css", "text/css; charset=utf-8"},
        {".js", "application/javascript; charset=utf-8"},
        {".json", "application/json; charset=utf-8"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".txt", "text/plain; charset=utf-8"},
        {".xml", "application/xml; charset=utf-8"},
        {".pdf", "application/pdf"},
    };

    auto it = mime_types.find(ext);
    return it != mime_types.end() ? std::string(it->second) : "application/octet-stream";
  }

  mutable std::mutex mutex_;
  std::unordered_map<std::string, CachedFile> cache_;
  std::size_t current_size_bytes_{0};
  std::size_t max_cache_size_bytes_;
};

} // namespace neonsignal
