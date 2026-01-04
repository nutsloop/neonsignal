#include "neonsignal/api_handler.h++"

#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener_helpers.h++"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

namespace neonsignal {

namespace {

std::string sanitize_filename_local(std::string_view raw) {
  std::filesystem::path p{std::string(raw)};
  auto name = p.filename().string();
  if (name.empty() || name == "." || name == "..") {
    return "upload.bin";
  }
  for (char &c : name) {
    if (c == '/' || c == '\\' || static_cast<unsigned char>(c) < 0x20) {
      c = '_';
    }
  }
  constexpr std::size_t kMaxLen = 255;
  if (name.size() > kMaxLen) {
    name.resize(kMaxLen);
  }
  return name;
}

std::filesystem::path make_unique_path_local(const std::filesystem::path &dir,
                                             const std::string &desired_name) {
  std::filesystem::path candidate = dir / desired_name;
  if (!std::filesystem::exists(candidate)) {
    return candidate;
  }
  std::filesystem::path stem = std::filesystem::path(desired_name).stem();
  std::string ext = std::filesystem::path(desired_name).extension().string();
  int counter = 1;
  while (true) {
    auto next = dir / (stem.string() + "_" + std::to_string(counter) + ext);
    if (!std::filesystem::exists(next)) {
      return next;
    }
    ++counter;
  }
}

} // namespace

bool ApiHandler::incoming_data(const std::shared_ptr<Http2Connection> &conn,
                                         std::uint32_t stream_id, const std::string &method,
                                         const std::string &upload_header_name,
                                          const std::string &path) {
  if (method != "POST") {
    std::string body = "Method Not Allowed";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 405, "text/plain; charset=utf-8", body_bytes);
    conn->events |= EPOLLOUT;
    loop_.update_fd(conn->fd, conn->events);
    return true; // handled (405)
  }
  std::error_code ec;
  const std::filesystem::path upload_dir("public/upload");
  std::filesystem::create_directories(upload_dir, ec);
  std::string safe_name = sanitize_filename_local(upload_header_name);
  auto fullpath = make_unique_path_local(upload_dir, safe_name);
  auto relpath = std::filesystem::path("/upload") / fullpath.filename();

  Http2Connection::StreamState st{};
  st.path = path;
  st.method = method;
  st.expect_body = true;
  st.is_upload = true;
  st.file_rel_path = relpath.string();
  st.file_full_path = fullpath.string();
  st.upload_name = safe_name;
  st.file.open(fullpath, std::ios::binary);
  if (!st.file.is_open()) {
    std::string body = "{\"error\":\"cannot open upload path\"}";
    std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
    build_response_frames(conn->write_buf, stream_id, 500, "application/json", body_bytes);
    conn->events |= EPOLLOUT;
    loop_.update_fd(conn->fd, conn->events);
    std::cerr << "UPLOAD init failed (open) fd=" << conn->fd << " stream=" << stream_id
              << " path=" << fullpath << '\n';
    return true; // handled (500)
  }
  // Boost per-stream and connection windows to keep uploads flowing.
  const std::uint32_t stream_window_boost = 32 * 1024 * 1024; // 32MB
  auto wu_stream = build_window_update(stream_id, stream_window_boost);
  auto wu_conn = build_window_update(0, stream_window_boost);
  conn->write_buf.insert(conn->write_buf.end(), wu_stream.begin(), wu_stream.end());
  conn->write_buf.insert(conn->write_buf.end(), wu_conn.begin(), wu_conn.end());
  conn->events |= EPOLLOUT;
  loop_.update_fd(conn->fd, conn->events);
  conn->streams[stream_id] = std::move(st);
  std::cerr << "UPLOAD init fd=" << conn->fd << " stream=" << stream_id << " method=" << method
            << " path=" << path << " target=" << fullpath << " name=" << safe_name << '\n';
  return true; // handled (stream registered)
}

} // namespace neonsignal
