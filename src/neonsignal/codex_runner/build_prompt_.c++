#include "neonsignal/codex_runner.h++"

#include <sstream>

namespace neonsignal {

std::string CodexRunner::build_prompt_(const CodexRecord &record) const {
  std::ostringstream out;
  out << "Title: " << record.title << "\n";
  out << "Meta tags: " << record.meta_tags << "\n";
  out << "Description:\n" << record.description << "\n";
  if (!record.file_refs.empty()) {
    out << "File references:\n";
    std::istringstream refs(record.file_refs);
    std::string line;
    while (std::getline(refs, line)) {
      if (line.empty()) {
        continue;
      }
      out << "- " << line << "\n";
    }
  }
  return out.str();
}

} // namespace neonsignal
