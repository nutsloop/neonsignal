#pragma once

#include "neonsignal/database.h++"

#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>

namespace neonsignal {

class CodexRunner {
public:
  explicit CodexRunner(Database &db);

  CodexRunner(const CodexRunner &) = delete;
  CodexRunner &operator=(const CodexRunner &) = delete;
  CodexRunner(CodexRunner &&) = delete;
  CodexRunner &operator=(CodexRunner &&) = delete;

  std::optional<CodexRun> start_run(std::string_view brief_id);

private:
  void run_async_(CodexRun run);
  std::string build_prompt_(const CodexRecord &record) const;
  std::string get_config_(std::string_view key, std::string_view fallback) const;
  std::uint64_t get_config_u64_(std::string_view key, std::uint64_t fallback) const;
  bool get_config_bool_(std::string_view key, bool fallback) const;
  std::string sanitize_filename_(std::string_view name) const;

  Database &db_;
  std::unordered_set<std::string> active_runs_;
  std::mutex active_mutex_;
};

} // namespace neonsignal
