#include "neonsignal/voltage_argv/help.h++"

#include <ansi.h++>

#include <format>

namespace neonsignal::voltage_argv {

std::string help::db_path_() const {
  using nutsloop::ansi;

  return std::format(
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  Path to the LIBMDBX embedded database file.\n"
      "  The parent directory must exist; the database file will be created if needed.\n\n"
      "{}\n"
      "  {}\n\n"
      "{}\n"
      "  data/neonsignal.mdb\n\n"
      "{}\n"
      "  {} spin --db-path=./data/neonsignal.mdb\n"
      "  {} spin --db-path=/var/lib/neonsignal/db.mdb\n",
      ansi("NAME").stylish().bold().str(), ansi("--db-path=<path>").bright_green().str(),
      ansi("DESCRIPTION").stylish().bold().str(), ansi("ENVIRONMENT").stylish().bold().str(),
      ansi("NEONSIGNAL_DB_PATH").bright_cyan().str(), ansi("DEFAULT").stylish().bold().str(),
      ansi("EXAMPLES").stylish().bold().str(), ansi("  <binary>").bright_yellow().str(),
      ansi("  <binary>").bright_yellow().str());
}

} // namespace neonsignal::voltage_argv
