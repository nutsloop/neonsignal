#include "neonsignal/logging.h++"

#include <pthread.h>

#include <cstddef>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace neonsignal {

namespace {

constexpr std::size_t kThreadNameMax = 64;

class ThreadPrefixBuf : public std::streambuf {
public:
  explicit ThreadPrefixBuf(std::streambuf *dest) : dest_(dest) {}

protected:
  int overflow(const int ch) override {
    if (ch == traits_type::eof()) {
      return sync() == 0 ? traits_type::not_eof(ch) : traits_type::eof();
    }
    auto &state = line_state();
    state.buffer_.push_back(static_cast<char>(ch));
    if (ch == '\n') {
      flush_line(state, true);
    }
    return ch;
  }

  int sync() override {
    auto &state = line_state();
    if (!state.buffer_.empty()) {
      flush_line(state, false);
      return 0;
    }
    return dest_->pubsync();
  }

private:
  struct LineState {
    std::string buffer_;
    bool line_open_{false};
  };

  static LineState &line_state() {
    static thread_local LineState state;
    return state;
  }

  void flush_line(LineState &state, const bool line_complete) {
    if (state.buffer_.empty()) {
      return;
    }
    std::string p;
    if (!state.line_open_) {
      p = prefix();
    }
    std::lock_guard<std::mutex> lock(mu_);
    if (!p.empty()) {
      dest_->sputn(p.data(), static_cast<std::streamsize>(p.size()));
    }
    dest_->sputn(state.buffer_.data(), static_cast<std::streamsize>(state.buffer_.size()));
    dest_->pubsync();
    state.buffer_.clear();
    state.line_open_ = !line_complete;
  }

  static std::string prefix() {
    char name[kThreadNameMax] = {};
    if (pthread_getname_np(pthread_self(), name, sizeof(name)) != 0 || name[0] == '\0') {
      std::ostringstream oss;
      oss << std::this_thread::get_id();
      return std::format("{}->", oss.str());
    }
    // If the name already contains a separator marker '>', use it as-is.
    if (const std::string_view sv{name}; sv.find('>') != std::string_view::npos) {
      return std::string{name};
    }
    return std::format("{}->", name);
  }

  std::streambuf *dest_;
  std::mutex mu_;
};

} // namespace

void install_thread_logging_prefix() {
  static ThreadPrefixBuf buf(std::cerr.rdbuf());
  static bool installed = false;
  if (!installed) {
    std::cerr.rdbuf(&buf);
    // Avoid per-insertion flush; ThreadPrefixBuf flushes whole lines atomically.
    std::cerr.unsetf(std::ios_base::unitbuf);
    char name[kThreadNameMax] = {};
    if (pthread_getname_np(pthread_self(), name, sizeof(name)) == 0 && name[0] == '\0') {
      pthread_setname_np(pthread_self(), "main");
    }
    installed = true;
  }
}

} // namespace neonsignal
