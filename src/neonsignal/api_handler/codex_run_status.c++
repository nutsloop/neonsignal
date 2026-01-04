#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <sstream>

namespace neonsignal {

namespace {

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

std::string query_param(std::string_view path, std::string_view key) {
  auto qpos = path.find('?');
  if (qpos == std::string_view::npos) {
    return {};
  }
  std::string_view query = path.substr(qpos + 1);
  while (!query.empty()) {
    auto amp = query.find('&');
    std::string_view chunk = query.substr(0, amp);
    auto eq = chunk.find('=');
    if (eq != std::string_view::npos) {
      auto k = chunk.substr(0, eq);
      auto v = chunk.substr(eq + 1);
      if (k == key) {
        return std::string(v);
      }
    }
    if (amp == std::string_view::npos) {
      break;
    }
    query.remove_prefix(amp + 1);
  }
  return {};
}

} // namespace

bool ApiHandler::codex_run_status(const std::shared_ptr<Http2Connection>& conn,
                                  std::uint32_t stream_id, const std::string& path) {
  auto id = query_param(path, "id");
  if (id.empty()) {
    std::string body = "{\"error\":\"missing-id\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 400, "application/json", body_bytes);
    conn->events |= EPOLLOUT;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }
  auto run = db_.fetch_codex_run(id);
  if (!run) {
    std::string body = "{\"error\":\"not-found\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 404, "application/json", body_bytes);
    conn->events |= EPOLLOUT;
    loop_.update_fd(conn->fd, conn->events);
    return true;
  }

  std::ostringstream body;
  body << "{";
  body << "\"id\":\"" << json_escape(run->id) << "\",";
  body << "\"brief_id\":\"" << json_escape(run->brief_id) << "\",";
  body << "\"status\":\"" << json_escape(run->status) << "\",";
  body << "\"message\":\"" << json_escape(run->message) << "\",";
  body << "\"cmdline\":\"" << json_escape(run->cmdline) << "\",";
  body << "\"last_message\":\"" << json_escape(run->last_message) << "\",";
  body << "\"created_at\":" << static_cast<std::uint64_t>(run->created_at) << ",";
  body << "\"started_at\":" << static_cast<std::uint64_t>(run->started_at) << ",";
  body << "\"finished_at\":" << static_cast<std::uint64_t>(run->finished_at) << ",";
  body << "\"exit_code\":" << run->exit_code << ",";
  body << "\"duration_ms\":" << run->duration_ms << ",";
  body << "\"stdout_bytes\":" << run->stdout_bytes << ",";
  body << "\"stderr_bytes\":" << run->stderr_bytes << ",";
  body << "\"artifact_count\":" << run->artifact_count;
  body << "}";

  auto text = body.str();
  std::vector<std::uint8_t> body_bytes(text.begin(), text.end());
  build_response_frames(conn->write_buf, stream_id, 200, "application/json", body_bytes);
  conn->events |= EPOLLOUT;
  loop_.update_fd(conn->fd, conn->events);
  return true;
}

} // namespace neonsignal
