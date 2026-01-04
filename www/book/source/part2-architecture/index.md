# Part 2: Core Architecture

This section provides a detailed look into the core architecture of the Neonsignal server, from its low-level event loop to the virtual hosting and configuration systems.

## Server Workflow

The Neonsignal server is built around a main process that owns the listen socket, a TLS context, an `epoll`-based `EventLoop`, and a map of active connections. A `ThreadPool` exists for optional blocking work, but all socket and HTTP/2 state management remains on the main thread.

The following diagram illustrates the detailed workflow:

```text
                          ┌─────────────────────────────────────┐
                          │  Main thread (neonsignal listener)  │
                          │  - creates TLS/HTTP2 listen socket  │
                          │  - registers listen fd in EventLoop │
                          └───────────────┬─────────────────────┘
                                          │ EPOLLIN (new client)
                                          ▼
┌─────────────────────────────────────────────────────────────────┐
│ EventLoop (epoll) on main thread                                 │
│ - dispatches fd events to callbacks                              │
│ - manages timers (redirect probe, etc.)                          │
└───────────────────────┬──────────────────────────────────────────┘
                        │ accept()
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ Accept callback (main thread)                                   │
│ - create Http2Connection { fd, SSL*, per-conn state }           │
│ - register conn fd EPOLLIN|EPOLLOUT in EventLoop                │
│ - TLS handshake + HTTP/2 parsing stay on main thread            │
└───────────────────────┬──────────────────────────────────────────┘
                        │ EPOLLIN/OUT per-connection
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ I/O handler (main thread, handle_io_)                           │
│ - TLS handshake (SSL_accept)                                    │
│ - HTTP/2 preface + SETTINGS/ACK                                 │
│ - HPACK decode headers                                          │
│ - auth gate for protected paths                                 │
│ - API dispatch (ApiHandler) or static/router                    │
│ - build HTTP/2 frames, queue to write_buf, set EPOLLOUT         │
│ - SSE/streams: mark flags and push DATA frames                  │
│ - uploads: accumulate DATA, write to disk, respond on END_STREAM│
└───────────────────────┬──────────────────────────────────────────┘
                        │ (optional blocking work)
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ ThreadPool (ns-pool-N workers)                                  │
│ - only for blocking/expensive tasks (e.g., heavy loads)         │
│ - workers prepare data; main thread owns sockets/state          │
│ - main thread queues frames and flips EPOLLOUT to flush         │
└─────────────────────────────────────────────────────────────────┘
```

### Expanded Flow

-   **Startup:** The main process creates a TLS listen socket with ALPN for HTTP/2, binds it, and registers the listen file descriptor with the `EventLoop`.
-   **Accept:** When a new client connects, the `handle_accept_` callback accepts the connection, wraps it in SSL, creates an `Http2Connection` object to hold its state, and registers the new client file descriptor with the `EventLoop`.
-   **Per-connection I/O (`handle_io_`):** This is the core of the request handling, all occurring on the main thread.
    1.  The TLS handshake is performed.
    2.  The HTTP/2 connection preface is validated.
    3.  A loop processes incoming HTTP/2 frames, decoding headers using HPACK.
    4.  **Auth Gate:** Protected paths are checked for a valid `ns_session` cookie. Unauthorized requests are redirected.
    5.  **API Dispatch:** API routes (e.g., for login, stats, SSE) are dispatched to the `ApiHandler`.
    6.  **Static/SPA:** If not an API route, the request is handled by the static file router. 404s for HTML paths serve the Single Page Application shell.
-   **Writing:** Outgoing frames are buffered. When the socket is ready for writing (`EPOLLOUT`), the buffered data is sent.
-   **Teardown:** On connection errors or closure, the `close_connection_` function cleans up all resources associated with the client.
