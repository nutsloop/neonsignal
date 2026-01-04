#include "neonsignal/database.h++"

#include <openssl/evp.h>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace neonsignal::db {

namespace {

std::string base64url_encode(std::span<const std::uint8_t> data) {
  static const char* kTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
  std::string out;
  out.reserve(((data.size() + 2) / 3) * 4);
  std::size_t i = 0;
  while (i + 2 < data.size()) {
    std::uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
    out.push_back(kTable[n & 63]);
    i += 3;
  }
  if (i + 1 == data.size()) {
    std::uint32_t n = (data[i] << 16);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
  } else if (i + 2 == data.size()) {
    std::uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
  }
  return out;
}

std::vector<std::uint8_t> base64url_decode(std::string_view input) {
  std::string s(input);
  std::replace(s.begin(), s.end(), '-', '+');
  std::replace(s.begin(), s.end(), '_', '/');
  while (s.size() % 4 != 0) {
    s.push_back('=');
  }
  int pad = 0;
  if (!s.empty() && s[s.size() - 1] == '=') ++pad;
  if (s.size() > 1 && s[s.size() - 2] == '=') ++pad;
  std::vector<unsigned char> buf((s.size() / 4) * 3 + 1);
  int len = EVP_DecodeBlock(buf.data(), reinterpret_cast<const unsigned char*>(s.data()),
                            static_cast<int>(s.size()));
  if (len < 0) {
    return {};
  }
  if (pad > 0 && len >= pad) {
    len -= pad;
  }
  return std::vector<std::uint8_t>(buf.begin(), buf.begin() + len);
}

std::string json_escape(std::string_view value) {
  std::string out;
  out.reserve(value.size());
  for (char c : value) {
    switch (c) {
      case '\\':
      case '"':
        out.push_back('\\');
        out.push_back(c);
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out.push_back(c);
        break;
    }
  }
  return out;
}

std::string extract_json_string(std::string_view json, std::string_view key) {
  auto pos = json.find("\"" + std::string(key) + "\"");
  if (pos == std::string_view::npos) {
    return {};
  }
  pos = json.find(':', pos);
  if (pos == std::string_view::npos) {
    return {};
  }
  pos = json.find('"', pos);
  if (pos == std::string_view::npos) {
    return {};
  }
  std::size_t end = json.find('"', pos + 1);
  if (end == std::string_view::npos) {
    return {};
  }
  return std::string(json.substr(pos + 1, end - pos - 1));
}

std::optional<std::uint64_t> extract_json_u64(std::string_view json, std::string_view key) {
  auto pos = json.find("\"" + std::string(key) + "\"");
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  pos = json.find(':', pos);
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  ++pos;
  while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
    ++pos;
  }
  std::size_t end = pos;
  while (end < json.size() && std::isdigit(static_cast<unsigned char>(json[end]))) {
    ++end;
  }
  if (end == pos) {
    return std::nullopt;
  }
  try {
    return static_cast<std::uint64_t>(std::stoull(std::string(json.substr(pos, end - pos))));
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<bool> extract_json_bool(std::string_view json, std::string_view key) {
  auto pos = json.find("\"" + std::string(key) + "\"");
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  pos = json.find(':', pos);
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  ++pos;
  while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
    ++pos;
  }
  if (pos + 4 <= json.size() && json.substr(pos, 4) == "true") {
    return true;
  }
  if (pos + 5 <= json.size() && json.substr(pos, 5) == "false") {
    return false;
  }
  return std::nullopt;
}

} // namespace

std::string user_to_json(const User& user) {
  std::ostringstream out;
  out << "{";
  out << "\"id\":" << user.id << ",";
  out << "\"email\":\"" << json_escape(user.email) << "\",";
  out << "\"display_name\":\"" << json_escape(user.display_name) << "\",";
  out << "\"verified\":" << (user.verified ? "true" : "false") << ",";
  if (!user.credential_id.empty()) {
    out << "\"credential_id\":\"" << base64url_encode(user.credential_id) << "\",";
  }
  if (!user.public_key.empty()) {
    out << "\"public_key\":\"" << base64url_encode(user.public_key) << "\",";
  }
  out << "\"sign_count\":" << user.sign_count << ",";
  out << "\"created_at\":" << static_cast<std::uint64_t>(user.created_at) << ",";
  out << "\"last_login\":" << static_cast<std::uint64_t>(user.last_login);
  out << "}";
  return out.str();
}

std::optional<User> user_from_json(std::string_view json) {
  User user;
  auto id = extract_json_u64(json, "id");
  auto email = extract_json_string(json, "email");
  auto display_name = extract_json_string(json, "display_name");
  auto verified = extract_json_bool(json, "verified");
  auto cred_id = extract_json_string(json, "credential_id");
  auto pk = extract_json_string(json, "public_key");
  auto sign = extract_json_u64(json, "sign_count");
  auto created = extract_json_u64(json, "created_at");
  auto last = extract_json_u64(json, "last_login");
  if (!id || email.empty()) {
    return std::nullopt;
  }
  user.id = *id;
  user.email = std::move(email);
  user.display_name = std::move(display_name);
  user.verified = verified.value_or(false);
  if (!cred_id.empty()) {
    user.credential_id = base64url_decode(cred_id);
  }
  if (!pk.empty()) {
    user.public_key = base64url_decode(pk);
  }
  user.sign_count = sign ? static_cast<std::uint32_t>(*sign) : 0;
  user.created_at = created ? static_cast<std::time_t>(*created) : 0;
  user.last_login = last ? static_cast<std::time_t>(*last) : 0;
  return user;
}

std::string session_to_json(const Session& session) {
  std::ostringstream out;
  out << "{";
  out << "\"user_id\":" << session.user_id << ",";
  out << "\"user\":\"" << json_escape(session.user) << "\",";
  out << "\"state\":\"" << json_escape(session.state) << "\",";
  out << "\"created_at\":" << static_cast<std::uint64_t>(session.created_at) << ",";
  out << "\"expires_at\":" << static_cast<std::uint64_t>(session.expires_at);
  out << "}";
  return out.str();
}

std::optional<Session> session_from_json(std::string_view json) {
  Session session;
  auto user_id = extract_json_u64(json, "user_id");
  auto user = extract_json_string(json, "user");
  auto state = extract_json_string(json, "state");
  auto created = extract_json_u64(json, "created_at");
  auto expires = extract_json_u64(json, "expires_at");
  if (!user_id || user.empty() || !created || !expires) {
    return std::nullopt;
  }
  session.user_id = *user_id;
  session.user = std::move(user);
  session.state = state.empty() ? "auth" : std::move(state);  // default to auth for legacy
  session.created_at = static_cast<std::time_t>(*created);
  session.expires_at = static_cast<std::time_t>(*expires);
  return session;
}

std::string verification_to_json(const Verification& v) {
  std::ostringstream out;
  out << "{";
  out << "\"user_id\":" << v.user_id << ",";
  out << "\"expires_at\":" << static_cast<std::uint64_t>(v.expires_at) << ",";
  out << "\"used_at\":" << static_cast<std::uint64_t>(v.used_at);
  out << "}";
  return out.str();
}

std::optional<Verification> verification_from_json(std::string_view json) {
  Verification v;
  auto user_id = extract_json_u64(json, "user_id");
  auto expires = extract_json_u64(json, "expires_at");
  auto used = extract_json_u64(json, "used_at");
  if (!user_id || !expires) {
    return std::nullopt;
  }
  v.user_id = *user_id;
  v.expires_at = static_cast<std::time_t>(*expires);
  v.used_at = used ? static_cast<std::time_t>(*used) : 0;
  return v;
}

std::string codex_to_json(const CodexRecord& record) {
  std::ostringstream out;
  out << "{";
  out << "\"id\":\"" << json_escape(record.id) << "\",";
  out << "\"content_type\":\"" << json_escape(record.content_type) << "\",";
  out << "\"sha256\":\"" << json_escape(record.sha256) << "\",";
  out << "\"size\":" << record.size << ",";
  out << "\"created_at\":" << static_cast<std::uint64_t>(record.created_at) << ",";
  out << "\"title\":\"" << json_escape(record.title) << "\",";
  out << "\"meta_tags\":\"" << json_escape(record.meta_tags) << "\",";
  out << "\"description\":\"" << json_escape(record.description) << "\",";
  out << "\"file_refs\":\"" << json_escape(record.file_refs) << "\",";
  out << "\"image_name\":\"" << json_escape(record.image_name) << "\",";
  out << "\"image_type\":\"" << json_escape(record.image_type) << "\",";
  out << "\"image_alt\":\"" << json_escape(record.image_alt) << "\",";
  out << "\"image_meta\":\"" << json_escape(record.image_meta) << "\",";
  out << "\"image_size\":" << record.image_size;
  out << "}";
  return out.str();
}

std::optional<CodexRecord> codex_from_json(std::string_view json) {
  CodexRecord record;
  auto id = extract_json_string(json, "id");
  auto content_type = extract_json_string(json, "content_type");
  auto sha256 = extract_json_string(json, "sha256");
  auto size = extract_json_u64(json, "size");
  auto created = extract_json_u64(json, "created_at");
  if (id.empty() || sha256.empty() || !size || !created) {
    return std::nullopt;
  }
  record.id = std::move(id);
  record.content_type = std::move(content_type);
  record.sha256 = std::move(sha256);
  record.size = *size;
  record.created_at = static_cast<std::time_t>(*created);
  record.title = extract_json_string(json, "title");
  record.meta_tags = extract_json_string(json, "meta_tags");
  record.description = extract_json_string(json, "description");
  record.file_refs = extract_json_string(json, "file_refs");
  record.image_name = extract_json_string(json, "image_name");
  record.image_type = extract_json_string(json, "image_type");
  record.image_alt = extract_json_string(json, "image_alt");
  record.image_meta = extract_json_string(json, "image_meta");
  auto image_size = extract_json_u64(json, "image_size");
  record.image_size = image_size ? *image_size : 0;
  return record;
}

std::string codex_run_to_json(const CodexRun& run) {
  std::ostringstream out;
  out << "{";
  out << "\"id\":\"" << json_escape(run.id) << "\",";
  out << "\"brief_id\":\"" << json_escape(run.brief_id) << "\",";
  out << "\"status\":\"" << json_escape(run.status) << "\",";
  out << "\"message\":\"" << json_escape(run.message) << "\",";
  out << "\"cmdline\":\"" << json_escape(run.cmdline) << "\",";
  out << "\"last_message\":\"" << json_escape(run.last_message) << "\",";
  out << "\"created_at\":" << static_cast<std::uint64_t>(run.created_at) << ",";
  out << "\"started_at\":" << static_cast<std::uint64_t>(run.started_at) << ",";
  out << "\"finished_at\":" << static_cast<std::uint64_t>(run.finished_at) << ",";
  out << "\"exit_code\":" << run.exit_code << ",";
  out << "\"duration_ms\":" << run.duration_ms << ",";
  out << "\"stdout_bytes\":" << run.stdout_bytes << ",";
  out << "\"stderr_bytes\":" << run.stderr_bytes << ",";
  out << "\"artifact_count\":" << run.artifact_count;
  out << "}";
  return out.str();
}

std::optional<CodexRun> codex_run_from_json(std::string_view json) {
  CodexRun run;
  run.id = extract_json_string(json, "id");
  run.brief_id = extract_json_string(json, "brief_id");
  run.status = extract_json_string(json, "status");
  run.message = extract_json_string(json, "message");
  run.cmdline = extract_json_string(json, "cmdline");
  run.last_message = extract_json_string(json, "last_message");
  auto created = extract_json_u64(json, "created_at");
  auto started = extract_json_u64(json, "started_at");
  auto finished = extract_json_u64(json, "finished_at");
  auto exit_code = extract_json_u64(json, "exit_code");
  auto duration = extract_json_u64(json, "duration_ms");
  auto stdout_bytes = extract_json_u64(json, "stdout_bytes");
  auto stderr_bytes = extract_json_u64(json, "stderr_bytes");
  auto artifact_count = extract_json_u64(json, "artifact_count");
  if (run.id.empty() || run.brief_id.empty() || run.status.empty() || !created) {
    return std::nullopt;
  }
  run.created_at = static_cast<std::time_t>(*created);
  run.started_at = started ? static_cast<std::time_t>(*started) : 0;
  run.finished_at = finished ? static_cast<std::time_t>(*finished) : 0;
  run.exit_code = exit_code ? static_cast<int>(*exit_code) : 0;
  run.duration_ms = duration ? *duration : 0;
  run.stdout_bytes = stdout_bytes ? *stdout_bytes : 0;
  run.stderr_bytes = stderr_bytes ? *stderr_bytes : 0;
  run.artifact_count = artifact_count ? *artifact_count : 0;
  return run;
}

} // namespace neonsignal::db
