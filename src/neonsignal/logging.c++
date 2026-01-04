#include "neonsignal/logging.h++"

#include <pthread.h>

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace neonsignal {

namespace {

class ThreadPrefixBuf : public std::streambuf {
public:
  explicit ThreadPrefixBuf(std::streambuf *dest) : dest_(dest) {}

protected:
  int overflow(const int ch) override {

    if (ch == traits_type::eof()) {
      return dest_->pubsync() == 0 ? traits_type::not_eof(ch) : traits_type::eof();
    }
    std::lock_guard<std::mutex> lock(mu_);
    if (at_line_start_) {
      const auto p = prefix();
      dest_->sputn(p.data(), static_cast<std::streamsize>(p.size()));
      at_line_start_ = false;
    }
    if (dest_->sputc(static_cast<char>(ch)) == traits_type::eof()) {
      return traits_type::eof();
    }
    if (ch == '\n') {
      at_line_start_ = true;
    }
    return ch;
  }

  int sync() override { return dest_->pubsync(); }

private:
  static std::string prefix() {
    char name[16] = {};
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
  bool at_line_start_{true};
  std::mutex mu_;
};

} // namespace

void install_thread_logging_prefix() {
  static ThreadPrefixBuf buf(std::cerr.rdbuf());
  static bool installed = false;
  if (!installed) {
    std::cerr.rdbuf(&buf);
    installed = true;
  }
}

} // namespace neonsignal
