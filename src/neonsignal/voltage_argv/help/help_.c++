#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::help_() const {
  using nutsloop::ansi;

  if (mode() == Mode::redirect) {
    return std::format(
        "{}\n{}\n\n"
        "{}\n"
        "  NeonSignal HTTP \u2192 HTTPS redirector service\n\n"
        "{}\n"
        "  {}          Start the redirector (required unless --systemd is used)\n\n"
        "{}\n"
        "  {}        Number of redirect service instances (default: 2)\n"
        "  {}           Bind address (default: 0.0.0.0)\n"
        "  {}              HTTP listen port (default: 9090)\n"
        "  {}       HTTPS target port (default: 443)\n"
        "  {}     ACME webroot directory for challenges\n"
        "  {}               Run in systemd mode (bypasses 'spin' requirement)\n\n"
        "{}\n"
        "  {}                  Show this help message\n"
        "  {}          Show help for specific option (e.g., --help=instances)\n"
        "  {}           Show version information\n\n"
        "{}\n"
        "  CLI flags are overridden by environment variables when set:\n"
        "    REDIRECT_INSTANCES, REDIRECT_HOST, REDIRECT_PORT\n"
        "    REDIRECT_TARGET_PORT, ACME_WEBROOT\n\n"
        "{}\n"
        "  {} spin --port=9090 --target-port=443\n"
        "  {} --systemd --instances=2\n"
        "  {} --help=acme-webroot\n",
        ansi("Usage:").stylish().bold().str(), ansi("  <binary> [OPTIONS] [COMMAND]").str(),
        ansi("Description:").stylish().bold().str(), ansi("Commands:").stylish().bold().str(),
        ansi("spin").bright_cyan().str(), ansi("→ Spin Options:").stylish().bold().str(),
        ansi("--instances=<n>").bright_green().str(), ansi("--host=<addr>").bright_green().str(),
        ansi("--port=<n>").bright_green().str(), ansi("--target-port=<n>").bright_green().str(),
        ansi("--acme-webroot=<path>").bright_green().str(), ansi("--systemd").bright_green().str(),
        ansi("Help:").stylish().bold().str(), ansi("--help").bright_cyan().str(),
        ansi("--help=<topic>").bright_cyan().str(), ansi("--version, -v").bright_cyan().str(),
        ansi("Environment Variables:").stylish().bold().str(),
        ansi("Examples:").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
        ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str());
  }

  return std::format(
      "{}\n{}\n\n"
      "{}\n"
      "  NeonSignal HTTP/2 server with TLS/SSL support\n\n"
      "{}\n"
      "  {}          Start the server (required unless --systemd is used)\n"
      "  {}       Install a repository into www-root\n\n"
      "{}\n"
      "  {}           Number of worker threads (default: 3)\n"
      "  {}           Bind address (default: 0.0.0.0)\n"
      "  {}              HTTPS listen port (default: 9443)\n"
      "  {} WebAuthn Relying Party ID\n"
      "  {} WebAuthn origin URL\n"
      "  {}        LIBMDBX database file path (default: data/neonsignal.mdb)\n"
      "  {}       Static files root directory (default: public)\n"
      "  {}     TLS certificates root directory (default: certs)\n"
      "  {}   Working directory for resolving paths\n"
      "  {}               Run in systemd mode (bypasses 'spin' requirement)\n\n"
      "{}\n"
      "  {}         Git repository URL (required unless --systemd-service is used)\n"
      "  {}     Target directory for clone (default: ./public)\n"
      "  {}          Custom directory name for the clone\n"
      "  {}        Specific branch to clone\n"
      "  {}   Install systemd service files (defaults when omitted; Linux only when installing)\n"
      "  {}   Save service files to a directory (default: current directory; requires --systemd-service)\n"
      "  ▲ Use --only-save to generate service files on non-Linux platforms\n\n"
      "{}\n"
      "  {}\n"
      "    {}\n"
      "  {}\n"
      "    {}\n"
      "    {}\n"
      "    {}\n"
      "    {}\n\n"
      "{}\n"
      "  {}                  Show this help message\n"
      "  {}          Show help for specific option (e.g., --help=threads)\n"
      "  {}           Show version information\n\n"
      "{}\n"
      "  CLI flags are overridden by environment variables when set:\n"
      "    NEONSIGNAL_THREADS, NEONSIGNAL_HOST, NEONSIGNAL_PORT\n"
      "    NEONSIGNAL_WEBAUTHN_DOMAIN, NEONSIGNAL_WEBAUTHN_ORIGIN\n"
      "    NEONSIGNAL_DB_PATH, NEONSIGNAL_WWW_ROOT, NEONSIGNAL_CERTS_ROOT\n"
      "    NEONSIGNAL_WORKING_DIR\n\n"
      "{}\n"
      "  {} spin --host=0.0.0.0 --port=9443\n"
      "  {} install --repo=https://github.com/user/repo.git\n"
      "  {} --systemd --threads=3\n"
      "  {} --help=webauthn-domain\n",
      ansi("Usage:").stylish().bold().str(), ansi("  <binary> [OPTIONS] [COMMAND]").str(),
      ansi("Description:").stylish().bold().str(), ansi("Commands:").stylish().bold().str(),
      ansi("spin").bright_cyan().str(), ansi("install").bright_cyan().str(),
      ansi("→ Spin Options:").stylish().bold().str(),
      ansi("--threads=<n>").bright_green().str(), ansi("--host=<addr>").bright_green().str(),
      ansi("--port=<n>").bright_green().str(), ansi("--webauthn-domain=<id>").bright_green().str(),
      ansi("--webauthn-origin=<url>").bright_green().str(),
      ansi("--db-path=<path>").bright_green().str(), ansi("--www-root=<path>").bright_green().str(),
      ansi("--certs-root=<path>").bright_green().str(), ansi("--working-dir=<path>").bright_green().str(),
      ansi("--systemd").bright_green().str(),
      ansi("Install Options:").stylish().bold().str(),
      ansi("--repo=<url>").bright_green().str(), ansi("--www-root=<path>").bright_green().str(),
      ansi("--name=<name>").bright_green().str(), ansi("--branch=<branch>").bright_green().str(),
      ansi("--systemd-service[=kvp]").bright_green().str(),
      ansi("--only-save[=<path>]").bright_green().str(),
      ansi("Install Switch Sets:").stylish().bold().str(),
      ansi("  <binary> install").bright_yellow().str(),
      ansi("--systemd-service").bright_green().str(),
      ansi("--only-save[=<path>]").bright_green().str(),
      ansi("  <binary> install").bright_yellow().str(),
      ansi("--repo").bright_green().str(), ansi("--www-root").bright_green().str(),
      ansi("--name").bright_green().str(), ansi("--branch").bright_green().str(),
      ansi("Help:").stylish().bold().str(), ansi("--help").bright_cyan().str(),
      ansi("--help=<topic>").bright_cyan().str(), ansi("--version, -v").bright_cyan().str(),
      ansi("Environment Variables:").stylish().bold().str(),
      ansi("Examples:").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
