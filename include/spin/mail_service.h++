#pragma once

#include "spin/database.h++"
#include "spin/mail_config.h++"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>

namespace neonsignal {

class MailService {
public:
  explicit MailService(Database& db, const MailConfig& config);

  MailService(const MailService&) = delete;
  MailService& operator=(const MailService&) = delete;
  MailService(MailService&&) = delete;
  MailService& operator=(MailService&&) = delete;

  struct MailRequest {
    std::string client_ip;
    std::string cookie_code;
    std::string form;
    std::string from;
    std::string name;
    std::string subject;
    std::string message;
  };

  std::optional<std::string> send_async(const MailRequest& request);

private:
  void send_async_(const MailSubmission& submission);
  std::uint32_t calculate_crc32(const std::string& data);

  const MailConfig& config_;
  Database& db_;
  std::mutex active_mutex_;
  std::unordered_set<std::string> active_sends_;
};

}  // namespace neonsignal
