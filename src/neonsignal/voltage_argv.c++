#include "neonsignal/voltage_argv.h++"
#include "neonsignal/voltage_argv/check.h++"
#include "neonsignal/voltage_argv/help.h++"

#include <args.h++>

#include <algorithm>
#include <array>
#include <cctype>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

namespace neonsignal {

using namespace nutsloop::args;

namespace {

using Sequencer = sequencer<voltage_argv::check, voltage_argv::help>;

template <std::size_t N>
bool contains_arg(const std::array<std::string_view, N> &list, std::string_view value) {
  return std::find(list.begin(), list.end(), value) != list.end();
}

// Parse version string "X.Y.Z" into version_t array {X, Y, Z}
constexpr version_t parse_version(std::string_view version_str) {

  version_t result = {0, 0, 0};

  std::size_t part = 0;
  std::size_t start = 0;

  for (std::size_t i = 0; i <= version_str.size() && part < 3; ++i) {
    if (i == version_str.size() || version_str[i] == '.') {
      if (i > start) {
        unsigned long value = 0;
        // Simple constexpr string to int conversion
        for (std::size_t j = start; j < i; ++j) {
          value = value * 10 + (version_str[j] - '0');
        }
        result[part++] = value;
      }
      start = i + 1;
    }
  }

  return result;
}

// Version constants from Meson build system
#ifndef NEONSIGNAL_VERSION
#define NEONSIGNAL_VERSION "0.2.0"
#endif

#ifndef NEONSIGNAL_REDIRECT_VERSION
#define NEONSIGNAL_REDIRECT_VERSION "0.1.0"
#endif

static constexpr version_t SERVER_VERSION = parse_version(NEONSIGNAL_VERSION);

static constexpr version_t REDIRECT_VERSION = parse_version(NEONSIGNAL_REDIRECT_VERSION);

// Valid flags for neonsignal
constexpr std::array<std::string_view, 14> server_args_list{
    {"threads", "host", "port", "webauthn-domain", "webauthn-origin", "db-path", "www-root",
     "certs-root", "working-dir", "systemd", "help", "?", "version", "v"}};

// Valid commands for neonsignal
constexpr std::array<std::string_view, 2> server_args_list_command{{"spin", "install"}};

// Valid flags for install command
constexpr std::array<std::string_view, 8> install_args_list{
    {"repo", "www-root", "name", "branch", "help", "?", "version", "v"}};

// Valid flags for neonsignal_redirect
constexpr std::array<std::string_view, 10> redirect_args_list{
    {"instances", "port", "target-port", "host", "acme-webroot", "systemd", "help", "?", "version",
     "v"}};

// Valid commands for neonsignal_redirect
constexpr std::array<std::string_view, 1> redirect_args_list_command{{"spin"}};

std::unordered_set<std::string> server_skip_digits() {
  return {"host", "webauthn-domain", "webauthn-origin", "db-path", "www-root", "certs-root", "working-dir"};
}

std::unordered_set<std::string> redirect_skip_digits() { return {"host", "acme-webroot"}; }

std::unordered_set<std::string> install_skip_digits() { return {"repo", "www-root", "name", "branch"}; }

[[noreturn]] void throw_invalid(const std::string &message) {
  throw std::invalid_argument(message);
}

} // namespace

// ───────────────────────────────────────────────────────────────────────────
// SERVER_VOLTAGE Implementation
// ───────────────────────────────────────────────────────────────────────────

server_voltage::server_voltage(int argc, char *argv[]) {

  voltage_argv::help::set_server_mode();
  validate_dash_format_(argc, argv);

  skip_digit_check_t skip{server_skip_digits()};
  Sequencer args(argc, argv, skip, SERVER_VERSION);

  // Validate flags against allowed list
  const auto parsed_args = args.get_args();
  for (const auto &[key, value] : parsed_args) {
    if (!contains_arg(server_args_list, key)) {
      throw_invalid(std::format("unknown flag: --{}", key));
    }
  }

  // Validate command against allowed list
  const auto command = args.get_command();
  if (!command.empty() && !contains_arg(server_args_list_command, command)) {
    throw_invalid(std::format("unknown command: '{}'", command));
  }

  // Handle help
  if (args.has("help") || args.has("?")) {
    show_help_ = true;
    auto topic =
        args.get_arg<std::string>("help").value_or(args.get_arg<std::string>("?").value_or("help"));
    auto helper = args.get_help(topic);
    help_text_ = helper.to_string();
    return;
  }

  // Handle version
  if (args.has("version") || args.has("v")) {
    show_version_ = true;
    auto helper = args.get_help("version");
    version_text_ = helper.to_string();
    return;
  }

  // Check systemd flag
  const bool systemd_flag = args.has("systemd") && args.get_option_bool("systemd").systemd();
  systemd_ = systemd_flag;

  // Validate command
  if (!systemd_flag) {
    if (command != "spin") {
      throw_invalid("command 'spin' is required when not running with --systemd");
    }
  } else if (!command.empty() && command != "spin") {
    throw_invalid("unexpected command when running with --systemd");
  }

  if (command.empty() && !systemd_flag) {
    throw_invalid("missing required command 'spin'");
  }

  // Parse options
  if (args.has("threads")) {
    threads_ = args.get_option_uint("threads").threads();
  }
  if (args.has("host")) {
    host_ = args.get_option_string("host").host();
  }
  if (args.has("port")) {
    port_ = args.get_option_uint("port").port();
  }
  if (args.has("webauthn-domain")) {
    webauthn_domain_ = args.get_option_string("webauthn-domain").webauthn_domain();
  }
  if (args.has("webauthn-origin")) {
    webauthn_origin_ = args.get_option_string("webauthn-origin").webauthn_origin();
  }
  if (args.has("db-path")) {
    db_path_ = args.get_option_string("db-path").db_path();
  }
  if (args.has("www-root")) {
    www_root_ = args.get_option_string("www-root").www_root();
  }
  if (args.has("certs-root")) {
    certs_root_ = args.get_option_string("certs-root").certs_root();
  }
  if (args.has("working-dir")) {
    working_dir_ = args.get_option_string("working-dir").working_dir();
  }
}

// ───────────────────────────────────────────────────────────────────────────
// REDIRECT_VOLTAGE Implementation
// ───────────────────────────────────────────────────────────────────────────

redirect_voltage::redirect_voltage(int argc, char *argv[]) {

  voltage_argv::help::set_redirect_mode();
  validate_dash_format_(argc, argv);

  skip_digit_check_t skip{redirect_skip_digits()};
  Sequencer args(argc, argv, skip, REDIRECT_VERSION);

  // Validate flags against allowed list
  const auto parsed_args = args.get_args();
  for (const auto &[key, value] : parsed_args) {
    if (!contains_arg(redirect_args_list, key)) {
      throw_invalid(std::format("unknown flag: --{}", key));
    }
  }

  // Validate command against allowed list
  const auto command = args.get_command();
  if (!command.empty() && !contains_arg(redirect_args_list_command, command)) {
    throw_invalid(std::format("unknown command: '{}'", command));
  }

  // Handle help
  if (args.has("help") || args.has("?")) {
    show_help_ = true;
    auto topic =
        args.get_arg<std::string>("help").value_or(args.get_arg<std::string>("?").value_or("help"));
    auto helper = args.get_help(topic);
    help_text_ = helper.to_string();
    return;
  }

  // Handle version
  if (args.has("version") || args.has("v")) {
    show_version_ = true;
    auto helper = args.get_help("version");
    version_text_ = helper.to_string();
    return;
  }

  // Check systemd flag
  const bool systemd_flag = args.has("systemd") && args.get_option_bool("systemd").systemd();
  systemd_ = systemd_flag;
  if (!systemd_flag) {
    if (command != "spin") {
      throw_invalid("command 'spin' is required when not running with --systemd");
    }
  } else if (!command.empty() && command != "spin") {
    throw_invalid("unexpected command when running with --systemd");
  }

  if (command.empty() && !systemd_flag) {
    throw_invalid("missing required command 'spin'");
  }

  // Parse options
  if (args.has("instances")) {
    instances_ = args.get_option_uint("instances").instances();
  }
  if (args.has("port")) {
    port_ = args.get_option_uint("port").redirect_port();
  }
  if (args.has("target-port")) {
    target_port_ = args.get_option_uint("target-port").target_port();
  }
  if (args.has("host")) {
    host_ = args.get_option_string("host").redirect_host();
  }
  if (args.has("acme-webroot")) {
    acme_webroot_ = args.get_option_string("acme-webroot").acme_webroot();
  }
}

// ───────────────────────────────────────────────────────────────────────────
// INSTALL_VOLTAGE Implementation
// ───────────────────────────────────────────────────────────────────────────

install_voltage::install_voltage(int argc, char *argv[]) {

  voltage_argv::help::set_server_mode();
  validate_dash_format_(argc, argv);

  skip_digit_check_t skip{install_skip_digits()};
  Sequencer args(argc, argv, skip, SERVER_VERSION);

  // Validate flags against allowed list
  const auto parsed_args = args.get_args();
  for (const auto &[key, value] : parsed_args) {
    if (!contains_arg(install_args_list, key)) {
      throw_invalid(std::format("unknown flag: --{}", key));
    }
  }

  // Validate command is 'install'
  const auto command = args.get_command();
  if (command != "install") {
    throw_invalid("install_voltage requires 'install' command");
  }

  // Handle help
  if (args.has("help") || args.has("?")) {
    show_help_ = true;
    auto topic =
        args.get_arg<std::string>("help").value_or(args.get_arg<std::string>("?").value_or("install"));
    auto helper = args.get_help(topic);
    help_text_ = helper.to_string();
    return;
  }

  // Handle version
  if (args.has("version") || args.has("v")) {
    show_version_ = true;
    auto helper = args.get_help("version");
    version_text_ = helper.to_string();
    return;
  }

  // Parse options
  if (args.has("repo")) {
    repo_ = args.get_option_string("repo").repo();
  }
  if (args.has("www-root")) {
    www_root_ = args.get_option_string("www-root").www_root();
  }
  if (args.has("name")) {
    name_ = args.get_option_string("name").install_name();
  }
  if (args.has("branch")) {
    branch_ = args.get_option_string("branch").branch();
  }

  // Validate required repo option
  if (!repo_) {
    throw_invalid("--repo is required for install command");
  }
}

} // namespace neonsignal
