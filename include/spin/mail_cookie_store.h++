#pragma once

#include "spin/mail_config.h++"

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

namespace neonsignal {

class MailCookieStore {
public:
  explicit MailCookieStore(const MailConfig& config);

  struct CookieEntry {
    std::string code;
    std::chrono::steady_clock::time_point expires_at;
  };

  std::string generate_and_store(const std::string& client_ip);
  bool validate(const std::string& client_ip, const std::string& code);
  void cleanup_expired();

private:
  std::string generate_sha256_code();

  const MailConfig& config_;
  std::mutex mutex_;
  std::unordered_map<std::string, CookieEntry> store_;
};

}  // namespace neonsignal
