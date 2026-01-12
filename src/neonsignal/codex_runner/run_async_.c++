#include "neonsignal/codex_runner.h++"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace neonsignal {

namespace {

std::vector<std::uint8_t> read_file_bytes(const std::filesystem::path &path,
                                          std::size_t max_bytes) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return {};
  }
  std::vector<std::uint8_t> bytes;
  file.seekg(0, std::ios::end);
  auto size = static_cast<std::size_t>(file.tellg());
  file.seekg(0, std::ios::beg);
  bytes.resize(std::min(size, max_bytes));
  if (!bytes.empty()) {
    file.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  }
  return bytes;
}

std::string read_file_text(const std::filesystem::path &path, std::size_t max_bytes) {
  auto bytes = read_file_bytes(path, max_bytes);
  return std::string(bytes.begin(), bytes.end());
}

std::string join_args(const std::vector<std::string> &args) {
  std::ostringstream out;
  bool first = true;
  for (const auto &arg : args) {
    if (!first) {
      out << ' ';
    }
    first = false;
    out << arg;
  }
  return out.str();
}

} // namespace

void CodexRunner::run_async_(CodexRun run) {
  auto record = db_.fetch_codex_record(run.brief_id);
  if (!record) {
    run.status = "failed";
    run.message = "brief not found";
    run.finished_at = std::time(nullptr);
    db_.update_codex_run(run);
    return;
  }

  auto payload = db_.fetch_codex_payload(run.brief_id);
  auto image_bytes = db_.fetch_codex_image(run.brief_id);

  run.status = "running";
  run.started_at = std::time(nullptr);
  db_.update_codex_run(run);

  auto start_time = std::chrono::steady_clock::now();
  std::filesystem::path run_dir = std::filesystem::path("data") / "codex" / "runs" / run.id;
  std::filesystem::create_directories(run_dir);

  auto prompt = build_prompt_(*record);
  auto prompt_path = run_dir / "prompt.txt";
  {
    std::ofstream out(prompt_path);
    out << prompt;
  }

  auto stdout_path = run_dir / "stdout.txt";
  auto stderr_path = run_dir / "stderr.txt";
  auto last_msg_path = run_dir / "last_message.txt";

  std::string codex_path = get_config_("codex_cli_path", "/usr/local/bin/codex");
  std::string profile = get_config_("codex_cli_profile", "");
  std::string workdir = get_config_("codex_cli_workdir", "");
  std::string output_schema = get_config_("codex_cli_output_schema", "");
  std::string color = get_config_("codex_cli_color", "auto");
  bool oss = get_config_bool_("codex_cli_oss", false);
  bool skip_git = get_config_bool_("codex_cli_skip_git", false);
  bool dry_run = get_config_bool_("codex_cli_dry_run", false);
  auto timeout_sec = get_config_u64_("codex_cli_timeout_sec", 120);
  auto max_output = get_config_u64_("codex_cli_max_output_bytes", 2 * 1024 * 1024);

  std::vector<std::string> args;
  args.push_back(codex_path);
  args.push_back("exec");
  args.push_back("--json");
  args.push_back("--output-last-message");
  args.push_back(last_msg_path.string());
  args.push_back("--yolo");
  // Newer codex exec versions reject --ask-for-approval, so keep flags minimal.
  if (!profile.empty()) {
    args.push_back("--profile");
    args.push_back(profile);
  }
  if (!workdir.empty()) {
    args.push_back("--cd");
    args.push_back(workdir);
  }
  if (!output_schema.empty()) {
    args.push_back("--output-schema");
    args.push_back(output_schema);
  }
  if (!color.empty()) {
    args.push_back("--color");
    args.push_back(color);
  }
  if (oss) {
    args.push_back("--oss");
  }
  if (skip_git) {
    args.push_back("--skip-git-repo-check");
  }

  std::optional<std::filesystem::path> image_path;
  if (image_bytes && !image_bytes->empty()) {
    auto fname = sanitize_filename_(record->image_name.empty() ? "image.bin" : record->image_name);
    image_path = run_dir / fname;
    std::ofstream out(*image_path, std::ios::binary);
    out.write(reinterpret_cast<const char *>(image_bytes->data()),
              static_cast<std::streamsize>(image_bytes->size()));
    args.push_back("--image");
    args.push_back(image_path->string());
  }

  args.push_back("--");
  args.push_back(prompt_path.string());
  run.cmdline = join_args(args);
  db_.update_codex_run(run);

  if (dry_run) {
    std::string fake = "dry-run: codex exec skipped";
    std::vector<std::uint8_t> stdout_bytes(fake.begin(), fake.end());
    db_.store_codex_run_streams(run.id, stdout_bytes, {});
    run.status = "completed";
    run.message = "dry-run";
    run.exit_code = 0;
    run.stdout_bytes = stdout_bytes.size();
    run.stderr_bytes = 0;
    run.last_message = fake;
    run.finished_at = std::time(nullptr);
    run.duration_ms = 0;
    db_.update_codex_run(run);
    db_.store_codex_run_artifacts(run.id, "[]");
    return;
  }

  int out_fd = ::open(stdout_path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
  int err_fd = ::open(stderr_path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
  if (out_fd == -1 || err_fd == -1) {
    if (out_fd != -1) {
      ::close(out_fd);
    }
    if (err_fd != -1) {
      ::close(err_fd);
    }
    run.status = "failed";
    run.message = "unable to open stdout/stderr files";
    run.finished_at = std::time(nullptr);
    db_.update_codex_run(run);
    return;
  }

  pid_t pid = fork();
  if (pid == 0) {
    ::dup2(out_fd, STDOUT_FILENO);
    ::dup2(err_fd, STDERR_FILENO);
    ::close(out_fd);
    ::close(err_fd);
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    for (auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);
    ::execvp(argv[0], argv.data());
    _exit(127);
  }

  ::close(out_fd);
  ::close(err_fd);

  bool timed_out = false;
  int status = 0;
  while (true) {
    pid_t res = ::waitpid(pid, &status, WNOHANG);
    if (res == pid) {
      break;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start_time);
    if (elapsed.count() >= static_cast<long>(timeout_sec)) {
      timed_out = true;
      ::kill(pid, SIGKILL);
      ::waitpid(pid, &status, 0);
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  auto stdout_bytes = read_file_bytes(stdout_path, max_output);
  auto stderr_bytes = read_file_bytes(stderr_path, max_output);
  db_.store_codex_run_streams(run.id, stdout_bytes, stderr_bytes);

  run.stdout_bytes = stdout_bytes.size();
  run.stderr_bytes = stderr_bytes.size();
  run.last_message = read_file_text(last_msg_path, max_output);

  run.exit_code = -1;
  if (timed_out) {
    run.status = "timeout";
    run.message = "timeout";
  } else if (WIFEXITED(status)) {
    run.exit_code = WEXITSTATUS(status);
    run.status = (run.exit_code == 0) ? "completed" : "failed";
  } else if (WIFSIGNALED(status)) {
    run.exit_code = 128 + WTERMSIG(status);
    run.status = "failed";
  } else {
    run.status = "failed";
  }

  auto finish_time = std::chrono::steady_clock::now();
  run.finished_at = std::time(nullptr);
  run.duration_ms = static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(finish_time - start_time).count());

  std::ostringstream artifacts;
  artifacts << "[";
  std::size_t count = 0;
  for (const auto &entry : std::filesystem::directory_iterator(run_dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    auto fname = entry.path().filename().string();
    if (fname == "stdout.txt" || fname == "stderr.txt" || fname == "prompt.txt" ||
        fname == "last_message.txt") {
      continue;
    }
    if (image_path && entry.path() == *image_path) {
      continue;
    }
    if (count > 0) {
      artifacts << ",";
    }
    artifacts << "{";
    artifacts << "\"name\":\"" << fname << "\",";
    artifacts << "\"size\":" << static_cast<std::uint64_t>(entry.file_size());
    artifacts << "}";
    ++count;
  }
  artifacts << "]";
  run.artifact_count = count;
  db_.store_codex_run_artifacts(run.id, artifacts.str());

  db_.update_codex_run(run);
}

} // namespace neonsignal
