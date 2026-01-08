#include "neonsignal/api_handler.h++"
#include "neonsignal/event_loop.h++"
#include "neonsignal/event_mask.h++"
#include "neonsignal/http2_listener.h++"
#include "neonsignal/http2_listener_api.h++"
#include "neonsignal/http2_listener_helpers.h++"
#include "neonsignal/routes.h++"

#if defined(__GLIBC__)
#include <malloc.h>
#endif
#include <openssl/ssl.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace neonsignal {

namespace {

std::optional<std::string>
extract_cookie(const std::unordered_map<std::string, std::string> &headers, std::string_view name) {
  auto it = headers.find("cookie");
  if (it == headers.end()) {
    return std::nullopt;
  }
  std::string_view cookie = it->second;
  std::string needle = std::string(name) + "=";
  std::size_t pos = cookie.find(needle);
  if (pos == std::string::npos) {
    return std::nullopt;
  }
  pos += needle.size();
  std::size_t end = cookie.find(';', pos);
  if (end == std::string::npos) {
    end = cookie.size();
  }
  return std::string(cookie.substr(pos, end - pos));
}

} // namespace

void Http2Listener::handle_io_(const std::shared_ptr<Http2Connection> &conn, std::uint32_t events) {
  if (conn->closed) {
    return;
  }

  if (events & (EventMask::Error | EventMask::HangUp)) {
    std::cerr << "Connection fd=" << conn->fd << " event error/hangup\n";
    close_connection_(conn->fd);
    return;
  }

  auto now = std::chrono::steady_clock::now();

  if (!conn->handshake_complete) {
    if (now > conn->handshake_deadline) {
      std::cerr << "TLS handshake timeout fd=" << conn->fd << '\n';
      close_connection_(conn->fd);
      return;
    }

    int ret = SSL_accept(conn->ssl.get());
    if (ret == 1) {
      conn->handshake_complete = true;
      conn->events = EventMask::Read | EventMask::Write;
      if (!conn->server_settings_sent) {
        auto settings = build_server_settings();
        conn->write_buf.insert(conn->write_buf.end(), settings.begin(), settings.end());
        conn->server_settings_sent = true;
      }
      // Increase connection window to allow larger uploads.
      const std::uint32_t conn_window_boost = 64 * 1024 * 1024; // 64MB
      auto wu_conn = build_window_update(0, conn_window_boost);
      conn->write_buf.insert(conn->write_buf.end(), wu_conn.begin(), wu_conn.end());
      loop_.update_fd(conn->fd, conn->events);
    } else {
      int err = SSL_get_error(conn->ssl.get(), ret);
      if (err == SSL_ERROR_WANT_READ) {
        conn->events = EventMask::Read;
      } else if (err == SSL_ERROR_WANT_WRITE) {
        conn->events = EventMask::Write;
      } else {
        std::cerr << "TLS handshake failed fd=" << conn->fd << " err=" << err << '\n';
        close_connection_(conn->fd);
        return;
      }
      loop_.update_fd(conn->fd, conn->events);
      return;
    }
  }

  if (events & EventMask::Read) {
    bool ok = true;
    for (;;) {
      std::uint8_t buf[4096];
      int n = SSL_read(conn->ssl.get(), buf, sizeof(buf));
      if (n > 0) {
        conn->read_buf.insert(conn->read_buf.end(), buf, buf + n);
      } else {
        int err = SSL_get_error(conn->ssl.get(), n);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
          break;
        }
        ok = false;
        break;
      }
    }

    if (!ok) {
      close_connection_(conn->fd);
      return;
    }

    // Preface check.
    if (!conn->preface_ok) {
      if (conn->read_buf.size() < kClientPreface.size()) {
        // Wait for full preface.
      } else {
        std::string_view got(reinterpret_cast<char *>(conn->read_buf.data()),
                             kClientPreface.size());
        if (got != kClientPreface) {
          std::cerr << "Invalid HTTP/2 preface on fd=" << conn->fd << '\n';
          close_connection_(conn->fd);
          return;
        }
        conn->preface_ok = true;
        conn->read_buf.erase(conn->read_buf.begin(),
                             conn->read_buf.begin() + kClientPreface.size());
        std::cerr << "Preface received fd=" << conn->fd << '\n';
      }
    }

    // Parse frames after preface.
    while (conn->preface_ok && conn->read_buf.size() >= 9) {
      std::uint32_t len = (conn->read_buf[0] << 16) | (conn->read_buf[1] << 8) | conn->read_buf[2];
      std::uint8_t type = conn->read_buf[3];
      std::uint8_t flags = conn->read_buf[4];
      std::uint32_t stream_id = ((conn->read_buf[5] & 0x7F) << 24) | (conn->read_buf[6] << 16) |
                                (conn->read_buf[7] << 8) | conn->read_buf[8];

      if (conn->read_buf.size() < 9 + len) {
        break;
      }

      std::vector<std::uint8_t> payload(conn->read_buf.begin() + 9,
                                        conn->read_buf.begin() + 9 + static_cast<std::size_t>(len));

      conn->read_buf.erase(conn->read_buf.begin(),
                           conn->read_buf.begin() + 9 + static_cast<std::size_t>(len));

      if (type == 0x4 /* SETTINGS */) {
        if ((flags & 0x1) == 0) {
          conn->client_settings_seen = true;
          auto ack = build_settings_ack();
          conn->write_buf.insert(conn->write_buf.end(), ack.begin(), ack.end());
          conn->events |= EventMask::Write;
          loop_.update_fd(conn->fd, conn->events);
          std::cerr << "Client SETTINGS received fd=" << conn->fd << '\n';
        }
        continue;
      }

      if (type == 0x1 /* HEADERS */ || type == 0x9 /* CONTINUATION */) {
        // Handle padding and priority on initial HEADERS only.
        std::size_t start = 0;
        if (type == 0x1 && (flags & 0x8)) { // PADDED
          if (payload.empty()) {
            close_connection_(conn->fd);
            return;
          }
          const std::uint8_t pad_len = payload[0];
          start = 1;
          // The bounds are checked before resizing to prevent an underflow.
          // The check `pad_len > payload.size() - 1` is equivalent to
          // `pad_len >= payload.size()` for non-empty payloads, which is
          // easier for some static analyzers to verify and removes the need for a pragma.
          if (static_cast<std::size_t>(pad_len) >= payload.size()) {
            close_connection_(conn->fd);
            return;
          }
          payload.resize(payload.size() - pad_len);
        }
        if (type == 0x1 && (flags & 0x20)) { // PRIORITY
          if (payload.size() < start + 5) {
            close_connection_(conn->fd);
            return;
          }
          start += 5; // skip priority fields
        }

        auto &block = conn->header_blocks[stream_id];
        if (start < payload.size()) {
          block.insert(block.end(), payload.begin() + static_cast<long>(start), payload.end());
        }

        if (flags & 0x4) { // END_HEADERS
          if (!conn->decoder) {
            conn->decoder = std::make_unique<HpackDecoder>();
          }
          auto parsed = conn->decoder->decode(block);
          block.clear();
          std::string path = "/";
          std::string method = "GET";
          std::string authority = "";
          std::unordered_map<std::string, std::string> header_map;
          std::string upload_header_name;
          if (parsed) {
            if (!parsed->path.empty()) {
              path = parsed->path;
            }
            if (!parsed->method.empty()) {
              method = parsed->method;
            }
            if (!parsed->authority.empty()) {
              authority = parsed->authority;
            }
            header_map = parsed->headers;
            auto it_name = parsed->headers.find("x-filename");
            if (it_name != parsed->headers.end()) {
              upload_header_name = it_name->second;
            }
          } else {
            std::cerr << "Missing or invalid headers, defaulting path=/ fd=" << conn->fd
                      << " stream=" << stream_id << '\n';
          }

          std::cerr << "HEADERS on fd=" << conn->fd << " stream=" << stream_id << " path=" << path
                    << " method=" << method << " authority=" << authority << '\n';

          conn->last_path = path;

          // SWITCH APIs and SPECIAL PATHS
          auto api_route = identify_api_route(path);

          // PROTECTED PATHS
          if (auth_.is_protected_path(path)) {
            std::string user;
            auto cookie = extract_cookie(header_map, "ns_session");
            auto build_auth_fail_headers = []() {
              return std::vector<std::pair<std::string, std::string>>{
                  {"set-cookie", "ns_session=; Path=/; Max-Age=0; HttpOnly; Secure; SameSite=Lax"},
                  {"set-cookie", "ns_debug=; Path=/; Max-Age=0; Secure; SameSite=Lax"}};
            };
            if (!cookie) {
              std::cerr << "auth: missing session cookie for path=" << path << '\n';
              std::string body = "{\"error\":\"auth-required\"}";
              std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
              if (api_route != ApiRoute::None) {
                auto hdrs = build_auth_fail_headers();
                build_response_frames_with_headers(conn->write_buf, stream_id, 500,
                                                   "application/json", hdrs, body_bytes);
              } else {
                std::string loc = std::string(routes::auth_redirect());
                auto hdrs = build_auth_fail_headers();
                hdrs.emplace_back("location", loc);
                build_response_frames_with_headers(conn->write_buf, stream_id, 302,
                                                   "application/json", hdrs, body_bytes);
              }
              conn->events |= EventMask::Write;
              loop_.update_fd(conn->fd, conn->events);
              continue;
            }

            // Check SessionCache first (60s TTL caching)
            bool valid = false;
            auto cached = session_cache_->get(*cookie);
            if (cached) {
              user = cached->user_id;
              valid = true;
              std::cerr << "auth: session cache HIT user=" << user << " path=" << path << '\n';
            } else {
              // Cache miss - validate with auth and cache result
              valid = auth_.validate_session(*cookie, user);
              if (valid) {
                auto now = std::chrono::steady_clock::now();
                session_cache_->put(*cookie, {.user_id = user,
                                              .credential_id = "",
                                              .cached_at = now,
                                              .expires_at = now + std::chrono::seconds(60),
                                              .authenticated = true});
                std::cerr << "auth: session cache MISS, validated user=" << user << " path=" << path
                          << '\n';
              }
            }

            if (!valid) {
              std::cerr << "auth: invalid session for path=" << path << " cookie=" << *cookie
                        << '\n';
              std::string body = "{\"error\":\"auth-required\"}";
              std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
              if (api_route != ApiRoute::None) {
                auto hdrs = build_auth_fail_headers();
                build_response_frames_with_headers(conn->write_buf, stream_id, 500,
                                                   "application/json", hdrs, body_bytes);
              } else {
                std::string loc = std::string(routes::auth_redirect());
                auto hdrs = build_auth_fail_headers();
                hdrs.emplace_back("location", loc);
                build_response_frames_with_headers(conn->write_buf, stream_id, 302,
                                                   "application/json", hdrs, body_bytes);
              }
              conn->events |= EventMask::Write;
              loop_.update_fd(conn->fd, conn->events);
              continue;
            }
          }

          bool handled_api = false;
          switch (api_route) {
          case ApiRoute::AuthLoginOptions:
            handled_api = api_handler_->auth_login_options(conn, stream_id);
            break;
          case ApiRoute::AuthLoginFinish:
            handled_api = api_handler_->auth_login_finish_headers(conn, stream_id, path, method);
            break;
          case ApiRoute::AuthUserCheck:
            handled_api = api_handler_->auth_user_check(conn, stream_id, header_map);
            break;
          case ApiRoute::AuthUserRegister:
            handled_api = api_handler_->user_register_headers(conn, stream_id, path, method);
            break;
          case ApiRoute::AuthUserVerify:
            handled_api = api_handler_->user_verify_headers(conn, stream_id, path, method);
            break;
          case ApiRoute::AuthUserEnroll:
            if (method == "GET") {
              handled_api = api_handler_->user_enroll(conn, stream_id, header_map);
            } else {
              handled_api = api_handler_->user_enroll_headers(conn, stream_id, header_map, path, method);
            }
            break;
          case ApiRoute::CodexBrief:
            handled_api = api_handler_->codex_brief_headers(conn, stream_id, header_map, path,
                                                            method);
            break;
          case ApiRoute::CodexList:
            handled_api = api_handler_->codex_list(conn, stream_id);
            break;
          case ApiRoute::CodexItem:
            handled_api = api_handler_->codex_item(conn, stream_id, path);
            break;
          case ApiRoute::CodexImage:
            handled_api = api_handler_->codex_image(conn, stream_id, path);
            break;
          case ApiRoute::CodexRunStart:
            handled_api = api_handler_->codex_run_start(conn, stream_id, path, method);
            break;
          case ApiRoute::CodexRunStatus:
            handled_api = api_handler_->codex_run_status(conn, stream_id, path);
            break;
          case ApiRoute::CodexRunStdout:
            handled_api = api_handler_->codex_run_stdout(conn, stream_id, path);
            break;
          case ApiRoute::CodexRunStderr:
            handled_api = api_handler_->codex_run_stderr(conn, stream_id, path);
            break;
          case ApiRoute::CodexRunArtifacts:
            handled_api = api_handler_->codex_run_artifacts(conn, stream_id, path);
            break;
          case ApiRoute::IncomingData:
            handled_api =
                api_handler_->incoming_data(conn, stream_id, method, upload_header_name, path);
            break;
          case ApiRoute::Stats:
            handled_api = api_handler_->stats(conn, stream_id, path, method, authority);
            break;
          case ApiRoute::Events:
            handled_api = api_handler_->events(conn, stream_id, path, method, authority);
            break;
          case ApiRoute::Cpu:
            handled_api = api_handler_->cpu_stream(conn, stream_id, path, method, authority);
            break;
          case ApiRoute::Memory:
            handled_api = api_handler_->memory_stream(conn, stream_id, path, method, authority);
            break;
          case ApiRoute::RedirectService:
            handled_api =
                api_handler_->redirect_service_stream(conn, stream_id, path, method, authority);
            break;
          default:
            break;
          }
          if (handled_api) {
            // Subscribe to SSE channels if stream was set up, and initialize counters.
            if (conn->is_event_stream) {
              conn->event_sse_count = 0;
              conn->event_sse_start = std::chrono::steady_clock::now();
              sse_broadcaster_->subscribe(SSEBroadcaster::Channel::Events, conn->fd, conn,
                                          conn->event_stream_id);
            }
            if (conn->is_cpu_stream) {
              conn->cpu_sse_count = 0;
              conn->cpu_sse_start = std::chrono::steady_clock::now();
              sse_broadcaster_->subscribe(SSEBroadcaster::Channel::CPUMetrics, conn->fd, conn,
                                          conn->cpu_stream_id);
            }
            if (conn->is_mem_stream) {
              conn->mem_sse_count = 0;
              conn->mem_sse_start = std::chrono::steady_clock::now();
              sse_broadcaster_->subscribe(SSEBroadcaster::Channel::MemMetrics, conn->fd, conn,
                                          conn->mem_stream_id);
            }
            if (conn->is_redirect_stream) {
              conn->redirect_sse_count = 0;
              conn->redirect_sse_start = std::chrono::steady_clock::now();
              sse_broadcaster_->subscribe(SSEBroadcaster::Channel::Redirect, conn->fd, conn,
                                          conn->redirect_stream_id);
            }
            continue;
          }

          auto is_html = routes::is_html_page(path) || path.find(".html") != std::string::npos;

          // Virtual host resolution - check if authority maps to a vhost directory
          StaticResult res;
          auto vhost_root = vhost_resolver_.resolve(authority);
          if (vhost_root) {
            // Use vhost-specific document root
            res = load_static_vhost(path, *vhost_root, router_);
          } else {
            // Fallback to default public root with cache
            res = load_static(path, router_, static_cache_.get());
          }

          // SPA routes - serve index.html shell with correct path for client-side routing
          auto load_shell = [&]() -> StaticResult {
            if (vhost_root) {
              return load_static_vhost(std::string(routes::pages::kIndex), *vhost_root, router_);
            }
            return load_static(std::string(routes::pages::kIndex), router_, static_cache_.get());
          };

          if (res.status == 404 && is_html && vhost_resolver_.is_neonjsx(authority)) {
            auto shell = load_shell();
            if (shell.status == 200) {
              res.content_type = shell.content_type;
              const bool is_known_route = vhost_resolver_.is_neonjsx_route(authority, path);
              res.status = is_known_route ? 200 : 404;
              std::string body_str(shell.body.begin(), shell.body.end());
              body_str += "<script>";
              if (!is_known_route) {
                body_str += "window.__NEON_STATUS=404;";
              }
              body_str += "window.__NEON_PATH=\"";
              body_str += path;
              body_str += "\";</script>";
              res.body.assign(body_str.begin(), body_str.end());
            }
          }

          // as you can see anytime the path:/upload goes it returns a 404 cause file is not there
          build_response_frames(conn->write_buf, stream_id, res.status, res.content_type, res.body);
          if (res.status == 200 && is_html) {
            ++page_views_;
          }
          ++served_files_;

          // Broadcast stats update to all Events channel subscribers
          auto files = served_files_.load();
          auto pages = page_views_.load();
          std::string event_body = "data: {\"files_served\": " + std::to_string(files) +
                                   ",\"page_views\": " + std::to_string(pages) + "}\n\n";
          std::vector<std::uint8_t> event_bytes(event_body.begin(), event_body.end());

          sse_broadcaster_->for_each_subscriber(
              SSEBroadcaster::Channel::Events,
              [&](const std::shared_ptr<Http2Connection> &c, std::uint32_t stream_id) {
                // Skip connections with write backpressure
                if (conn_manager_->has_write_backpressure(c)) {
                  return;
                }
                auto data_frame = build_frame(0x0 /* DATA */, 0x0, stream_id, event_bytes);
                c->write_buf.insert(c->write_buf.end(), data_frame.begin(), data_frame.end());
                c->events |= EventMask::Write;
                loop_.update_fd(c->fd, c->events);
              });
          conn->events |= EventMask::Write;
          loop_.update_fd(conn->fd, conn->events);
          if (path == routes::pages::kHome || path == routes::pages::kIndex) {
            std::cerr << "Serving index.html\n";
          }
          std::cerr << "HEADERS on fd=" << conn->fd << " path=" << path << " method=" << method
                    << " authority=" << authority << '\n';
        }
        continue;
      }

      if (type == 0x0 /* DATA */) {
        auto st_it = conn->streams.find(stream_id);
        if (st_it == conn->streams.end()) {
          continue;
        }

        auto &st = st_it->second;
        if (!st.expect_body || st.responded) {
          continue;
        }

        constexpr std::uint64_t kMaxUploadBytes =
            128ull * 1024ull * 1024ull * 1024ull; // 128 GB cap
        if (st.is_upload) {
          st.received_bytes += static_cast<std::uint64_t>(payload.size());
          if (st.received_bytes > kMaxUploadBytes) {
            std::string body = "Payload too large";
            std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
            build_response_frames(conn->write_buf, stream_id, 413, "text/plain; charset=utf-8",
                                  body_bytes);
            if (st.file.is_open()) {
              st.file.close();
              std::error_code ec;
              std::filesystem::remove(st.file_full_path, ec);
            }
            conn->events |= EventMask::Write;
            loop_.update_fd(conn->fd, conn->events);
            st.responded = true;
            std::cerr << "UPLOAD too large fd=" << conn->fd << " stream=" << stream_id
                      << " size=" << st.received_bytes << '\n';
            continue;
          }
          if (!st.write_failed) {
            st.file.write(reinterpret_cast<const char *>(payload.data()),
                          static_cast<std::streamsize>(payload.size()));
            if (!st.file.good()) {
              st.write_failed = true;
              std::cerr << "UPLOAD write failed fd=" << conn->fd << " stream=" << stream_id << '\n';
            }
          }
        } else {
          if (st.body.size() + payload.size() > kMaxUploadBytes) {
            std::string body = "Payload too large";
            std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
            build_response_frames(conn->write_buf, stream_id, 413, "text/plain; charset=utf-8",
                                  body_bytes);
            conn->events |= EventMask::Write;
            loop_.update_fd(conn->fd, conn->events);
            st.responded = true;
            continue;
          }
          st.body.insert(st.body.end(), payload.begin(), payload.end());
        }
        if (flags & 0x1 /* END_STREAM */) {
          int status = 201;
          std::string content_type = "application/json";
          std::vector<std::uint8_t> body_bytes;
          std::string saved_path;

          if (st.is_upload) {
            if (st.file.is_open()) {
              st.file.close();
            }
            if (st.write_failed) {
              status = 500;
              std::string err = "{\"error\":\"failed to persist upload\"}";
              body_bytes.assign(err.begin(), err.end());
            } else {
              saved_path = st.file_rel_path;
              std::string ok = "{\"status\":\"stored\",\"path\":\"" + saved_path +
                               "\",\"bytes\":" + std::to_string(st.received_bytes) + "}";
              body_bytes.assign(ok.begin(), ok.end());
              status = 201;
            }
          } else {
            std::string body_str(st.body.begin(), st.body.end());
            if (st.is_auth_finish) {
              auto auth_res = auth_.finish_login(body_str);
              if (!auth_res.ok) {
                status = 401;
                std::string err = "{\"error\":\"" + auth_res.error + "\"}";
                body_bytes.assign(err.begin(), err.end());
                build_response_frames(conn->write_buf, stream_id, status, content_type, body_bytes);
                conn->events |= EventMask::Write;
                loop_.update_fd(conn->fd, conn->events);
                st.responded = true;
                conn->streams.erase(st_it);
                continue;
              }
              std::string ok = "{\"status\":\"ok\",\"user\":\"" + auth_res.user + "\"}";
              body_bytes.assign(ok.begin(), ok.end());
              // Host-only cookie (no Domain) to avoid user agent rejection with
              // self-signed certs; secure/httponly to protect the session.
              // 5-day session lifetime.
              std::string cookie = "ns_session=" + auth_res.session_id +
                                   "; Path=/; Max-Age=432000; HttpOnly; Secure; SameSite=Lax";
              // Debug cookie (non-HttpOnly) to verify browser storage behavior.
              std::string dbg_cookie = "ns_debug=" + auth_res.session_id +
                                       "; Path=/; Max-Age=432000; Secure; SameSite=Lax";
              std::cerr << "auth: issued session for user=" << auth_res.user
                        << " session=" << auth_res.session_id << '\n';
              build_response_frames_with_headers(
                  conn->write_buf, stream_id, 200, content_type,
                  {{"set-cookie", cookie}, {"set-cookie", dbg_cookie}}, body_bytes);
              conn->events |= EventMask::Write;
              loop_.update_fd(conn->fd, conn->events);
              st.responded = true;
              conn->streams.erase(st_it);
              continue;
            } else if (st.is_register_finish) {
              auto reg_res = auth_.finish_register(body_str, st.reg_secret);
              if (!reg_res.ok) {
                status = 400;
                std::string err = "{\"error\":\"" + reg_res.error + "\"}";
                body_bytes.assign(err.begin(), err.end());
              } else {
                std::string ok = "{\"status\":\"registered\"}";
                body_bytes.assign(ok.begin(), ok.end());
                status = 201;
              }
            } else if (st.is_user_register) {
              auto response = api_handler_->user_register_finish(st.body);
              status = response.status;
              content_type = response.content_type;
              body_bytes = std::move(response.body);
            } else if (st.is_user_verify) {
              auto response = api_handler_->user_verify_finish(st.body);
              status = response.status;
              content_type = response.content_type;
              body_bytes = std::move(response.body);
              // If successful, set session cookie
              if (status == 200) {
                std::string resp_str(body_bytes.begin(), body_bytes.end());
                auto pos = resp_str.find("\"session_id\":\"");
                if (pos != std::string::npos) {
                  pos += 14;
                  auto end = resp_str.find('"', pos);
                  if (end != std::string::npos) {
                    std::string session_id = resp_str.substr(pos, end - pos);
                    std::string cookie = "ns_session=" + session_id +
                                         "; Path=/; Max-Age=300; HttpOnly; Secure; SameSite=Lax";
                    std::string dbg_cookie = "ns_debug=" + session_id +
                                             "; Path=/; Max-Age=300; Secure; SameSite=Lax";
                    build_response_frames_with_headers(
                        conn->write_buf, stream_id, status, content_type,
                        {{"set-cookie", cookie}, {"set-cookie", dbg_cookie}}, body_bytes);
                    conn->events |= EventMask::Write;
                    loop_.update_fd(conn->fd, conn->events);
                    st.responded = true;
                    conn->streams.erase(st_it);
                    continue;
                  }
                }
              }
            } else if (st.is_user_enroll) {
              auto response = api_handler_->user_enroll_finish(st.session_user_id, st.body);
              status = response.status;
              content_type = response.content_type;
              body_bytes = std::move(response.body);
              // If successful, set new auth session cookie
              if (status == 200) {
                std::string resp_str(body_bytes.begin(), body_bytes.end());
                auto pos = resp_str.find("\"session_id\":\"");
                if (pos != std::string::npos) {
                  pos += 14;
                  auto end = resp_str.find('"', pos);
                  if (end != std::string::npos) {
                    std::string session_id = resp_str.substr(pos, end - pos);
                    std::string cookie = "ns_session=" + session_id +
                                         "; Path=/; Max-Age=432000; HttpOnly; Secure; SameSite=Lax";
                    std::string dbg_cookie = "ns_debug=" + session_id +
                                             "; Path=/; Max-Age=432000; Secure; SameSite=Lax";
                    build_response_frames_with_headers(
                        conn->write_buf, stream_id, status, content_type,
                        {{"set-cookie", cookie}, {"set-cookie", dbg_cookie}}, body_bytes);
                    conn->events |= EventMask::Write;
                    loop_.update_fd(conn->fd, conn->events);
                    st.responded = true;
                    conn->streams.erase(st_it);
                    continue;
                  }
                }
              }
            } else if (st.is_codex) {
              auto ct = st.content_type.empty() ? "application/octet-stream"
                                                : std::string_view(st.content_type);
              auto response = api_handler_->codex_brief_finish(ct, st.body);
              status = response.status;
              content_type = response.content_type;
              body_bytes = std::move(response.body);
            } else {
              std::string ok = "{\"status\":\"ok\"}";
              body_bytes.assign(ok.begin(), ok.end());
            }
          }

          build_response_frames(conn->write_buf, stream_id, status, content_type, body_bytes);
          ++served_files_;
          conn->events |= EventMask::Write;
          loop_.update_fd(conn->fd, conn->events);
          st.responded = true;
          std::cerr << "UPLOAD complete fd=" << conn->fd << " stream=" << stream_id << " bytes="
                    << (st.is_upload ? st.received_bytes
                                     : static_cast<std::uint64_t>(st.body.size()))
                    << " status=" << status;
          if (!saved_path.empty()) {
            std::cerr << " path=" << saved_path;
          }
          std::cerr << '\n';
          conn->streams.erase(st_it);
        } else {
          // Advance windows by the amount received.
          std::uint32_t delta =
              static_cast<std::uint32_t>(std::min<std::size_t>(payload.size(), 0x7FFFFFFFu));
          auto wu_stream = build_window_update(stream_id, delta);
          auto wu_conn = build_window_update(0, delta);
          conn->write_buf.insert(conn->write_buf.end(), wu_stream.begin(), wu_stream.end());
          conn->write_buf.insert(conn->write_buf.end(), wu_conn.begin(), wu_conn.end());
          conn->events |= EventMask::Write;
          loop_.update_fd(conn->fd, conn->events);
        }
      }
    }
  }

  if (events & EventMask::Write) {
    auto now = std::chrono::steady_clock::now();
    static std::chrono::steady_clock::time_point last_trim{};
    if (last_trim.time_since_epoch().count() == 0 || (now - last_trim) >= std::chrono::minutes(5)) {
      // Best-effort memory trim to return free pages to OS.
      last_trim = now;
#if defined(__GLIBC__)
      (void)malloc_trim(0);
#endif
    }
    auto should_reset = [&](bool is_stream, std::uint64_t count,
                            std::chrono::steady_clock::time_point start) {
      if (!is_stream) {
        return false;
      }
      if (start.time_since_epoch().count() == 0) {
        return false;
      }
      bool time_exceeded = (sse_policy_.mode == SSEResetPolicy::Mode::OnlyCount)
                               ? false
                               : (now - start >= sse_policy_.max_age);
      bool count_exceeded = (sse_policy_.mode == SSEResetPolicy::Mode::OnlyTime)
                                ? false
                                : (count >= sse_policy_.max_messages);
      if (sse_policy_.mode == SSEResetPolicy::Mode::Both) {
        return time_exceeded || count_exceeded;
      }
      return time_exceeded || count_exceeded;
    };

    auto reset_stream = [&](bool &flag, std::uint32_t stream_id, std::uint64_t &count,
                            std::chrono::steady_clock::time_point &start) {
      auto data_frame = build_frame(0x0 /* DATA */, 0x1 /* END_STREAM */, stream_id, {});
      conn->write_buf.insert(conn->write_buf.end(), data_frame.begin(), data_frame.end());
      flag = false;
      count = 0;
      start = {};
      conn->events |= EventMask::Write;
    };

    while (conn->write_offset < conn->write_buf.size()) {
      int n = SSL_write(conn->ssl.get(), conn->write_buf.data() + conn->write_offset,
                        static_cast<int>(conn->write_buf.size() - conn->write_offset));
      if (n > 0) {
        conn->write_offset += static_cast<std::size_t>(n);
        continue;
      }
      int err = SSL_get_error(conn->ssl.get(), n);
      if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
        break;
      }
      std::cerr << "SSL_write error fd=" << conn->fd << " err=" << err << '\n';
      close_connection_(conn->fd);
      return;
    }

    if (conn->write_offset >= conn->write_buf.size()) {
      conn->write_buf.clear();
      conn->write_offset = 0;
      conn->events = EventMask::Read;
      if (conn->is_event_stream) {
        if (conn->event_sse_start.time_since_epoch().count() == 0) {
          conn->event_sse_start = now;
        }
        if (should_reset(conn->is_event_stream, conn->event_sse_count, conn->event_sse_start)) {
          reset_stream(conn->is_event_stream, conn->event_stream_id, conn->event_sse_count,
                       conn->event_sse_start);
          loop_.update_fd(conn->fd, conn->events);
          return;
        }
        // Throttle events: send every 2 seconds.
        constexpr auto kEventInterval = std::chrono::seconds(2);
        if (conn->event_sse_start.time_since_epoch().count() == 0 ||
            (now - conn->event_sse_start) >= kEventInterval) {
          conn->event_sse_start = now;
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          std::string body = "data: {\"files_served\": " + std::to_string(served_files_.load()) +
                             ",\"page_views\": " + std::to_string(page_views_.load()) + "}\n\n";
          std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
          auto data_frame = build_frame(0x0 /* DATA */, 0x0, conn->event_stream_id, body_bytes);
          conn->write_buf.insert(conn->write_buf.end(), data_frame.begin(), data_frame.end());
          conn->events |= EventMask::Write;
          ++conn->event_sse_count;
        }
      }
      if (conn->is_cpu_stream) {
        if (conn->cpu_sse_start.time_since_epoch().count() == 0) {
          conn->cpu_sse_start = now;
        }
        if (should_reset(conn->is_cpu_stream, conn->cpu_sse_count, conn->cpu_sse_start)) {
          reset_stream(conn->is_cpu_stream, conn->cpu_stream_id, conn->cpu_sse_count,
                       conn->cpu_sse_start);
          loop_.update_fd(conn->fd, conn->events);
          return;
        }
        // Throttle CPU updates: send every 5 seconds.
        constexpr auto kCpuInterval = std::chrono::seconds(5);
        if (conn->cpu_sse_start.time_since_epoch().count() == 0 ||
            (now - conn->cpu_sse_start) >= kCpuInterval) {
          conn->cpu_sse_start = now;
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          timespec ts{};
          clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
          std::uint64_t cpu_now = static_cast<std::uint64_t>(ts.tv_sec) * 1000000000ull +
                                  static_cast<std::uint64_t>(ts.tv_nsec);
          auto wall_now = std::chrono::steady_clock::now();
          double percent = 0.0;
          if (conn->last_cpu_time_ns != 0 && conn->last_cpu_wall.time_since_epoch().count() != 0) {
            auto cpu_delta = cpu_now - conn->last_cpu_time_ns;
            auto wall_delta =
                std::chrono::duration_cast<std::chrono::nanoseconds>(wall_now - conn->last_cpu_wall)
                    .count();
            if (wall_delta > 0) {
              percent = (static_cast<double>(cpu_delta) / static_cast<double>(wall_delta)) * 100.0;
            }
          }
          conn->last_cpu_time_ns = cpu_now;
          conn->last_cpu_wall = wall_now;

          std::string body = "data: {\"cpu_percent\": " + std::to_string(percent) + "}\n\n";
          std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
          auto data_frame = build_frame(0x0 /* DATA */, 0x0, conn->cpu_stream_id, body_bytes);
          conn->write_buf.insert(conn->write_buf.end(), data_frame.begin(), data_frame.end());
          conn->events |= EventMask::Write;
          ++conn->cpu_sse_count;
        }
      }
      if (conn->is_mem_stream) {
        if (conn->mem_sse_start.time_since_epoch().count() == 0) {
          conn->mem_sse_start = now;
        }
        if (should_reset(conn->is_mem_stream, conn->mem_sse_count, conn->mem_sse_start)) {
          reset_stream(conn->is_mem_stream, conn->mem_stream_id, conn->mem_sse_count,
                       conn->mem_sse_start);
          loop_.update_fd(conn->fd, conn->events);
          return;
        }
        // Throttle memory updates: send every 1 minute.
        constexpr auto kMemInterval = std::chrono::minutes(1);
        if (conn->mem_sse_start.time_since_epoch().count() == 0 ||
            (now - conn->mem_sse_start) >= kMemInterval) {
          conn->mem_sse_start = now;
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          std::uint64_t rss = read_rss_kb();
          std::string body = "data: {\"rss_kb\": " + std::to_string(rss) + "}\n\n";
          std::vector<std::uint8_t> body_bytes(body.begin(), body.end());
          auto data_frame = build_frame(0x0 /* DATA */, 0x0, conn->mem_stream_id, body_bytes);
          conn->write_buf.insert(conn->write_buf.end(), data_frame.begin(), data_frame.end());
          conn->events |= EventMask::Write;
          conn->last_rss_kb = rss;
          ++conn->mem_sse_count;
        }
      }
      if (conn->is_redirect_stream) {
        if (conn->redirect_sse_start.time_since_epoch().count() == 0) {
          conn->redirect_sse_start = now;
        }
        if (should_reset(conn->is_redirect_stream, conn->redirect_sse_count,
                         conn->redirect_sse_start)) {
          reset_stream(conn->is_redirect_stream, conn->redirect_stream_id, conn->redirect_sse_count,
                       conn->redirect_sse_start);
          loop_.update_fd(conn->fd, conn->events);
          return;
        }
      }
      loop_.update_fd(conn->fd, conn->events);
    } else {
      conn->events = EventMask::Read | EventMask::Write;
      loop_.update_fd(conn->fd, conn->events);
    }
  }
}

} // namespace neonsignal
