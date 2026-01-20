#include "spin/codex_runner.h++"

#include <chrono>

namespace neonsignal {

std::optional<CodexRun> CodexRunner::start_run(std::string_view brief_id) {
  if (brief_id.empty()) {
    return std::nullopt;
  }

  auto record = db_.fetch_codex_record(brief_id);
  if (!record) {
    return std::nullopt;
  }

  std::string cmdline = get_config_("codex_cli_path", "/usr/local/bin/codex");
  cmdline += " exec";

  auto run = db_.create_codex_run(brief_id, cmdline);
  if (!run) {
    return std::nullopt;
  }

  {
    std::lock_guard<std::mutex> lock(active_mutex_);
    active_runs_.insert(run->id);
  }

  std::thread([this, run_id = run->id]() {
    auto run_meta = db_.fetch_codex_run(run_id);
    if (run_meta) {
      run_async_(*run_meta);
    }
    std::lock_guard<std::mutex> lock(active_mutex_);
    active_runs_.erase(run_id);
  }).detach();

  return run;
}

} // namespace neonsignal
