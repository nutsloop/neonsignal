# OSx Review

## Review Snapshot

### Project Map
- Monorepo combines a C++23 HTTP/2 server, custom JSX runtime, multiple frontend apps, and docs. Core code lives in `src/` and `include/` with web assets under `www/` and `public/`.
- Entry points are `src/main.c++` (TLS HTTP/2 server) and `src/redirect_main.c++` (HTTP redirector). UI bootstraps from `www/neonsignaljsx/main.tsx`.
- Server defaults are in `include/neonsignal/neonsignal.h++` with env overrides in `src/neonsignal/server/run.c++`.
- Core deps are OpenSSL, nghttp2, libmdbx (subproject fallback), wired in `meson.build` and `src/meson.build`.
- Node tooling and lint config live in `package.json`, `tsconfig.json`, and `eslint.config.js`.

### Backend Runtime
- Server initializes TLS and SNI via CertManager, then spins EventLoop, ThreadPool, Router, and Http2Listener. See `src/neonsignal/server/initialize_tls.c++` and `src/neonsignal/http2_listener.c++`.
- Event loop uses epoll or kqueue with timers and signal handling. See `include/neonsignal/event_loop_backend.h++`, `src/neonsignal/event_loop/linux/backend.c++`, and `src/neonsignal/event_loop/darwin/backend.c++`.
- HTTP/2 is parsed manually (preface, SETTINGS, HEADERS/CONTINUATION, DATA), and responses are framed and HPACK-encoded in house. See `src/neonsignal/http2_listener/handle_io_.c++` and `src/neonsignal/http2_listener/helper/build_response_frames.c++`.
- Header decompression uses nghttp2 HPACK inflater. See `include/neonsignal/hpack_decoder.h++` and `src/neonsignal/hpack_decoder/decode.c++`.
- Static routing uses `Router` with directory traversal guard, plus in-memory caching and vhost-aware document roots. See `src/neonsignal/router/resolve.c++`, `include/neonsignal/static_cache.h++`, and `src/neonsignal/vhost/resolve.c++`.
- SSE and metrics streams are supported over HTTP/2, and a separate redirect service handles HTTP to HTTPS and ACME. See `include/neonsignal/sse_broadcaster.h++`, `src/neonsignal/api_handler/events.c++`, and `include/neonsignal/redirect_service.h++`.

### Data, Auth, Codex
- LibMDBX stores users, sessions, verifications, codex metadata and payloads, and config. See `include/neonsignal/database.h++` and `src/neonsignal/database/database.c++`.
- Serialization is custom JSON + base64url helpers. See `src/neonsignal/database/serialization.c++`.
- WebAuthn flows are implemented in house (challenge issuance, ES256 verify, COSE to SPKI). See `include/neonsignal/webauthn.h++` and `src/neonsignal/webauthn.c++`.
- Registration flow creates a pending user and verification token (logged to console) with a demo cap of one user. See `src/neonsignal/api_handler/user_register.c++` and `src/neonsignal/api_handler/user_verify.c++`.
- API routing enumerates auth, codex, stats, and SSE endpoints and stream state handling. See `include/neonsignal/routes.h++` and `src/neonsignal/api_handler/identify_api_route.c++`.
- Codex runner forks `codex exec`, captures stdout, stderr, and artifacts, and persists metadata. See `include/neonsignal/codex_runner.h++` and `src/neonsignal/codex_runner/run_async_.c++`.

### Frontend and Tooling
- Custom JSX runtime is a small VDOM renderer with `h` and `Fragment` globals and no diffing. See `neonjsx/runtime.ts`.
- Neonsignal UI bootstraps client-side routing and auth gating in `www/neonsignaljsx/main.tsx`.
- SSE client code uses EventSource with throttling and gear modes. See `www/neonsignaljsx/scripts/sse/controller.ts`.
- Build scripts transpile with Babel and bundle with esbuild. See `scripts/neonsignaljsx/build.sh` and `scripts/nutsloopjsx/build.sh`.
- Docs pipeline uses Sphinx sources in `www/book` and scripts under `scripts/sphinx/`.
- Release script builds C++ and frontends in one pass. See `scripts/build-release.sh`.

### Observations and Gaps
- Upload endpoint `/api/incoming_data` is not protected and writes to `public/upload`, which allows unauthenticated disk usage and public file hosting. See `include/neonsignal/routes.h++` and `src/neonsignal/api_handler/incoming_data.c++`.
- Event loop can block on cache misses and includes small sleeps in SSE write paths, which can reduce responsiveness under load. See `src/neonsignal/http2_listener/helper/load_static.c++` and `src/neonsignal/http2_listener/handle_io_.c++`.
- Resource limits in `include/neonsignal/connection_manager.h++` are not enforced in the upload path, and the runtime cap in `src/neonsignal/http2_listener/handle_io_.c++` is 128GB.
- Session cache cleanup and SSE batch flushing helpers are declared but unused, so caches may grow and batching remains dormant. See `include/neonsignal/session_cache.h++` and `include/neonsignal/http2_listener.h++`.
- HTTP/2 support focuses on a minimal subset (no GOAWAY, PING, RST_STREAM handling observed), which can limit interoperability. See `src/neonsignal/http2_listener/handle_io_.c++`.
- No automated tests and some docs or config paths appear environment-specific. See `scripts/global_variables.sh`.

## HTTP/2 State Machine and Flow Control (Detailed)

### Connection Lifecycle
- Accepted: `handle_accept_` accepts and enqueues `handle_connection_`. See `src/neonsignal/http2_listener/handle_accept_.c++`.
- TLS handshake: `handle_connection_` creates SSL, sets nonblocking and `handshake_deadline` (now + 5s), and registers the fd. See `src/neonsignal/http2_listener/handle_connection_.c++`.
- Handshake loop: `handle_io_` calls `SSL_accept` until success or timeout. On success it queues server SETTINGS and a connection WINDOW_UPDATE (+64MB). See `src/neonsignal/http2_listener/handle_io_.c++`.
- Preface gate: once handshake completes, `handle_io_` waits for the client preface `PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n`. If mismatch, the connection is closed.
- Frame loop: once `preface_ok` is true, the read buffer is parsed into 9 byte frame headers plus payload. Only SETTINGS, HEADERS, CONTINUATION, and DATA frames are handled.
- Close: errors, hangups, handshake timeout, or invalid preface call `close_connection_`, which removes the fd, unsubscribes SSE, and closes the socket. See `src/neonsignal/http2_listener/close_connection_.c++`.

### Stream Lifecycle
- HEADERS or CONTINUATION: payload fragments are appended to `header_blocks[stream_id]`. Padding and priority are stripped for HEADERS.
- END_HEADERS: HPACK decode runs via `HpackDecoder`; path, method, and headers are extracted. See `src/neonsignal/hpack_decoder/decode.c++`.
- Routing: API routes are matched first, then static files and SPA shell fallback. See `src/neonsignal/http2_listener/handle_io_.c++` and `include/neonsignal/routes.h++`.
- Request body handling: for endpoints that need a body, a `StreamState` is stored with `expect_body = true`. Uploads write to disk, other bodies accumulate in memory.
- END_STREAM on DATA: finishes upload or API processing, builds response frames, sets `responded`, and erases the stream from `conn->streams`.
- There is no explicit handling of RST_STREAM, stream error states, or server-initiated stream IDs. Streams are tracked only by map entries and the `responded` flag.

### Flow Control and Backpressure
- Server SETTINGS advertise `SETTINGS_MAX_CONCURRENT_STREAMS = 100` and `SETTINGS_INITIAL_WINDOW_SIZE = 16MB`. See `src/neonsignal/http2_listener/helper/build_server_settings.c++`.
- After TLS handshake, the server sends a connection WINDOW_UPDATE of +64MB.
- For `/api/incoming_data`, the server boosts stream and connection windows by +32MB at upload start.
- On each DATA frame received, the server sends WINDOW_UPDATE for both stream and connection with `increment = payload.size()`.
- The code does not track remote flow-control windows or apply send-side window checks. Outgoing backpressure is effectively SSL_write blocking and `ConnectionManager::MAX_WRITE_BUFFER_BYTES` checks (used only for SSE broadcast gating).
- SETTINGS updates for MAX_FRAME_SIZE and window size are not honored, and there is no enforcement of `MAX_STREAMS_PER_CONNECTION` or `MAX_READ_BUFFER_BYTES`.

### Timeout and Cleanup Notes
- `handshake_deadline` is checked in `handle_io_`, and `ConnectionManager::find_timed_out_connections()` is called every 5 seconds. See `src/neonsignal/http2_listener/start.c++` and `include/neonsignal/connection_manager.h++`.
- `Http2Connection::last_activity` is never updated after construction, so idle timeouts can close active connections after the fixed timeout window.

## Security Hardening Review (Auth, Uploads, Codex Runner)

### Critical
- Auth bypass via email-only verification: `user_verify_finish` issues a `pre_webauthn` session when the token is empty but the email is verified, enabling anyone who knows the email to enroll a new credential and take over the account. See `src/neonsignal/api_handler/user_verify.c++` and `src/neonsignal/api_handler/user_enroll.c++`.

### High
- Protected endpoints accept any session state: `validate_session` does not enforce `state == "auth"`, so `pre_webauthn` sessions can access protected paths (including codex and metrics). See `src/neonsignal/webauthn.c++` and `src/neonsignal/http2_listener/handle_io_.c++`.
- Unauthenticated file upload: `/api/incoming_data` is not protected and writes directly into `public/upload`, allowing public file hosting, disk exhaustion, and stored XSS. See `include/neonsignal/routes.h++` and `src/neonsignal/api_handler/incoming_data.c++`.
- Codex runner can be abused for resource exhaustion: no concurrency limits, runs can be spawned repeatedly, and stdout or stderr can grow without bound on disk. See `src/neonsignal/codex_runner/run_async_.c++`.

### Medium
- Codex brief payloads are stored without size limits and can allocate large in-memory buffers before the 128GB cap triggers. See `src/neonsignal/api_handler/codex_brief.c++` and `src/neonsignal/http2_listener/handle_io_.c++`.
- Session cache can accept a session beyond its DB expiration for up to its cache TTL. See `include/neonsignal/session_cache.h++`.

### Low
- Debug cookie `ns_debug` is not HttpOnly, which can leak the session id to XSS or injected scripts. See `src/neonsignal/http2_listener/handle_io_.c++`.
- Cookie parsing is substring-based and does not enforce cookie boundaries, which can misread similarly named cookies. See `src/neonsignal/http2_listener/handle_io_.c++` and `src/neonsignal/api_handler/user_enroll.c++`.

### Suggested Mitigations (Focused)
- Remove or lock down the email-only verification branch. Require a valid verification token or an authenticated session with a privileged state.
- Enforce session state for protected paths: only allow `state == "auth"` in `validate_session` or during route protection checks.
- Protect uploads behind auth and enforce a much lower size cap. Consider storing uploads outside `public/` and serving via a controlled handler.
- Add codex run limits (per user and global), restrict who can start runs, and enforce output size limits using pipes or `ulimit`.
- Drop `ns_debug` in production builds, and harden cookie parsing to handle boundaries safely.

## Targeted Test Plan (Routing, HPACK, SSE)

### Routing
- Router resolves `/` to `index.html` and strips query strings. Exercise `Router::resolve` with `"/?a=1"` and `"/index.html?x=y"`.
- Directory paths resolve to default document. Use a temp directory with nested `index.html`.
- Path traversal attempts with `..` are rejected. Verify `"/../secret"` returns `found = false`.
- Vhost resolution is case-insensitive and strips port. Test `example.com:443` and `EXAMPLE.COM`.
- NeonJSX routing from `.neonjsx` config handles exact and wildcard routes. Verify `is_neonjsx_route` for `"/docs/*"` style entries.

### HPACK
- Build a header block with nghttp2 deflater and ensure `HpackDecoder::decode` returns expected `:path`, `:method`, `:authority`, `:scheme`, and custom headers.
- Validate cookie merging: two `cookie` header fields should join with `"; "` in the decoded map.
- Invalid or truncated header blocks return `nullopt`.

### SSE
- Unit test `SSEBroadcaster` subscriptions and `flush_batched` to ensure combined payload uses `"\n\n"` separators and the DATA frame length field matches the payload.
- Verify `unsubscribe` and `unsubscribe_all` remove subscribers and stop delivery.
- Add a small helper test to verify SSE payload formatting for `events`, `cpu`, and `memory` channels (string content only, no network).

### Manual Checks
- `curl -k --http2 https://localhost:9443/` for basic HTTP/2 response.
- `curl -k --http2 https://localhost:9443/api/events` to verify SSE headers and streaming.
- `h2load -n 1000 -c 10 https://localhost:9443/` for load behavior (manual only).
