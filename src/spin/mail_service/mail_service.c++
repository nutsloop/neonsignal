#include "spin/mail_service.h++"

#include <chrono>
#include <format>
#include <string>

namespace neonsignal {

MailService::MailService(Database& db, const MailConfig& config)
    : config_(config), db_(db) {}

std::optional<std::string> MailService::send_async(const MailRequest& request) {
  if (!config_.enabled) {
    return std::nullopt;
  }

  if (request.client_ip.empty() || request.cookie_code.empty() || request.from.empty() ||
      request.message.empty()) {
    return std::nullopt;
  }

  if (config_.from_addresses.empty() || config_.mail_command.empty()) {
    return std::nullopt;
  }

  auto now = std::chrono::system_clock::now();
  auto now_time = std::chrono::system_clock::to_time_t(now);
  auto atomic_time = static_cast<std::uint64_t>(now.time_since_epoch().count());

  std::string composite = request.client_ip + request.cookie_code +
                          std::to_string(atomic_time);
  std::uint32_t crc = calculate_crc32(composite);
  std::string id = std::format("{:08x}", crc);

  MailSubmission submission;
  if (config_.save_to_database) {
    submission.n = db_.get_next_mail_submission_n();
  } else {
    submission.n = 0;
  }
  submission.id = id;
  submission.ip = request.client_ip;
  submission.date = now_time;
  submission.form = request.form;
  submission.from = request.from;

  std::string to = submission.from;
  for (const auto& extra : config_.to_extra) {
    if (!to.empty()) {
      to += ", ";
    }
    to += std::string(extra);
  }
  submission.to = std::move(to);

  submission.subject = request.subject;
  submission.body = request.message;
  submission.status = "queued";

  if (config_.save_to_database) {
    if (!db_.create_mail_submission(submission)) {
      return std::nullopt;
    }
  }

  {
    std::lock_guard<std::mutex> lock(active_mutex_);
    if (!active_sends_.insert(submission.id).second) {
      return std::nullopt;
    }
  }

  send_async_(submission);
  return submission.id;
}

}  // namespace neonsignal
