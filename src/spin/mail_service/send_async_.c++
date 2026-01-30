#include "spin/mail_service.h++"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <string_view>
#include <thread>
#include <vector>

namespace neonsignal {

void MailService::send_async_(const MailSubmission& submission) {
  std::thread([this, submission]() {
    std::vector<std::string> recipients;
    if (!submission.from.empty()) {
      recipients.push_back(submission.from);
    }
    for (const auto& extra : config_.to_extra) {
      recipients.emplace_back(extra);
    }

    std::string from_addr = config_.from_addresses.empty() ? "" : config_.from_addresses.front();

    auto is_msmtp_command = [](std::string_view cmd) {
      if (auto pos = cmd.find_last_of('/'); pos != std::string_view::npos) {
        cmd = cmd.substr(pos + 1);
      }
      return cmd == "msmtp" || cmd == "msmtpq";
    };

    const bool use_msmtp = is_msmtp_command(config_.mail_command);

    std::vector<std::string> args = {config_.mail_command};
    if (use_msmtp) {
      args.push_back("-t");
    } else {
      args.push_back("-s");
      args.push_back(submission.subject);
      args.push_back("-r");
      args.push_back(from_addr);
      for (const auto& r : recipients) {
        args.push_back(r);
      }
    }

    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (auto& arg : args) {
      argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    std::string payload;
    if (use_msmtp) {
      std::string to_header;
      for (std::size_t i = 0; i < recipients.size(); ++i) {
        if (i > 0) {
          to_header += ", ";
        }
        to_header += recipients[i];
      }
      payload = "From: " + from_addr + "\n";
      payload += "To: " + to_header + "\n";
      payload += "Subject: " + submission.subject + "\n";
      if (!submission.from.empty()) {
        payload += "Reply-To: " + submission.from + "\n";
      }
      payload += "\n";
      payload += submission.body;
    } else {
      payload = submission.body;
    }

    bool ok = false;
    int pipefd[2];
    if (pipe(pipefd) == 0) {
      pid_t pid = fork();
      if (pid == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execvp(argv[0], argv.data());
        _exit(127);
      }

      if (pid > 0) {
        close(pipefd[0]);
        const char* data = payload.data();
        std::size_t remaining = payload.size();
        while (remaining > 0) {
          ssize_t n = write(pipefd[1], data, remaining);
          if (n <= 0) {
            break;
          }
          data += n;
          remaining -= static_cast<std::size_t>(n);
        }
        close(pipefd[1]);

        int status = 0;
        if (waitpid(pid, &status, 0) == pid) {
          ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
        }
      } else {
        close(pipefd[0]);
        close(pipefd[1]);
      }
    }

    if (config_.save_to_database) {
      db_.update_mail_submission_status(submission.id, ok ? "sent" : "failed");
    }

    std::lock_guard<std::mutex> lock(active_mutex_);
    active_sends_.erase(submission.id);
  }).detach();
}

}  // namespace neonsignal
