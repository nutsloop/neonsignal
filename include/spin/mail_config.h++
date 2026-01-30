#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace neonsignal {

#ifdef MAIL_COMMAND
#define MAIL_COMMAND_STR_IMPL(x) #x
#define MAIL_COMMAND_STR(x) MAIL_COMMAND_STR_IMPL(x)
inline constexpr std::string_view kDefaultMailCommand = MAIL_COMMAND_STR(MAIL_COMMAND);
#undef MAIL_COMMAND_STR
#undef MAIL_COMMAND_STR_IMPL
#else
inline constexpr std::string_view kDefaultMailCommand = "mail";
#endif

struct MailConfig {
  bool enabled{false};
  std::vector<std::string> allowed_domains{"10.0.0.6:8889"};
  std::string cookie_name{"neon-mail"};
  std::chrono::seconds cookie_lifespan{900};
  std::vector<std::string> url_hits{"/contact.html", "/enroll.html"};
  std::vector<std::string> from_addresses{"noreply@example.com"};
  std::vector<std::string> to_extra{"contact@example.com", "support@example.com"};
  std::string mail_command{std::string(kDefaultMailCommand)};
  std::string allowed_ip_address{};
  bool save_to_database{true};
};

}  // namespace neonsignal
