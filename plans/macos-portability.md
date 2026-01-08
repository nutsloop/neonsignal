MacOS Portability Plan
======================

Goal: Make the C++ server build and run on macOS while preserving Linux
behavior. The focus is on event loop portability, socket setup, signal
handling, and build configuration.


Design Decision: Abstraction vs Conditional Compilation
-------------------------------------------------------

Two approaches are viable:

Option A: Full backend abstraction (recommended)
  - Clean separation with linux/ and darwin/ subdirectories
  - No #ifdef scattered through business logic
  - Easier to test each backend in isolation
  - Aligns with NeonSignal's "custom everything" philosophy
  - More files, more indirection

Option B: Inline #ifdef __APPLE__ blocks
  - Simpler for small deltas
  - Keeps related code together
  - Fewer files to maintain
  - Can become messy as platform differences grow

Recommendation: Use Option A (full abstraction) for event loop, timers, and
signals. Use Option B (inline #ifdef) for small one-off differences like
pthread_setname_np and MSG_NOSIGNAL.


Inventory of Linux-only APIs
----------------------------

Event loop and I/O:
- epoll: include/neonsignal/http2_listener.h++, src/neonsignal/event_loop.c++,
  src/neonsignal/event_loop/*.c++, src/neonsignal/http2_listener/*.c++,
  src/neonsignal/redirect_service/*.c++, src/neonsignal/server/run.c++
- timerfd: src/neonsignal/event_loop/add_timer.c++,
  src/neonsignal/http2_listener/start_redirect_monitor_.c++
- signalfd: src/neonsignal/server/run.c++

Socket operations:
- accept4 and SOCK_NONBLOCK/SOCK_CLOEXEC: src/neonsignal/http2_listener/handle_accept_.c++,
  src/neonsignal/redirect_service/handle_accept_.c++,
  src/neonsignal/http2_listener/helper/make_listen_socket.c++,
  src/neonsignal/redirect_service/setup_listener_.c++
- MSG_NOSIGNAL: src/neonsignal/redirect_service/handle_io_.c++

Threading:
- pthread_setname_np signature mismatch:
  src/redirect_main.c++, src/neonsignal/thread_pool.c++, src/neonsignal/logging.c++

Items to verify (likely portable, confirm):
- libmdbx: Should build on macOS, verify meson finds it correctly
- /proc filesystem: Search codebase for any /proc/self/* usage (none expected)
- OpenSSL: Homebrew installs to non-standard paths (see Step 6)


Step 1: Event flag abstraction and header cleanup
-------------------------------------------------

- Introduce OS-neutral event flags (READ, WRITE, ERR, HUP, EDGE) in a new
  header such as include/neonsignal/event_mask.h++ or in
  include/neonsignal/event_loop.h++.
- Replace EPOLL* usage with new flags in:
  include/neonsignal/http2_listener.h++,
  src/neonsignal/http2_listener/handle_io_.c++,
  src/neonsignal/http2_listener/start.c++,
  src/neonsignal/redirect_service/*.c++,
  src/neonsignal/server/run.c++,
  src/neonsignal/api_handler/*.c++
- Remove <sys/epoll.h> from include/neonsignal/http2_listener.h++ and keep
  OS headers private to backend implementations.

EDGE-TRIGGER SEMANTICS WARNING:
  EPOLLET (Linux) and EV_CLEAR (macOS kqueue) are similar but not identical:
  - EPOLLET: Edge-triggered, requires draining fd until EAGAIN
  - EV_CLEAR: Resets state after event delivery, re-arms automatically

  The current codebase likely drains correctly (required for EPOLLET), so
  EV_CLEAR should work. However, verify these files handle partial reads:
  - src/neonsignal/http2_listener/handle_io_.c++
  - src/neonsignal/redirect_service/handle_io_.c++

  Test case: Large request body that arrives in multiple chunks.


Step 2: Split EventLoop backends (epoll vs kqueue)
--------------------------------------------------

- Create a backend interface (e.g., include/neonsignal/event_loop_backend.h++).
- Refactor include/neonsignal/event_loop.h++ to store a backend pointer
  instead of epoll_fd_/timer_fd_ members.
- Move Linux implementation into new files:
  src/neonsignal/event_loop/linux/backend.c++,
  src/neonsignal/event_loop/linux/add_fd.c++,
  src/neonsignal/event_loop/linux/update_fd.c++,
  src/neonsignal/event_loop/linux/remove_fd.c++,
  src/neonsignal/event_loop/linux/run.c++,
  src/neonsignal/event_loop/linux/add_timer.c++
- Add macOS kqueue implementation in:
  src/neonsignal/event_loop/darwin/backend.c++,
  src/neonsignal/event_loop/darwin/add_fd.c++,
  src/neonsignal/event_loop/darwin/update_fd.c++,
  src/neonsignal/event_loop/darwin/remove_fd.c++,
  src/neonsignal/event_loop/darwin/run.c++,
  src/neonsignal/event_loop/darwin/add_timer.c++
- Keep the public EventLoop methods in the existing per-method files
  (src/neonsignal/event_loop/*.c++) as thin delegators to the backend to
  preserve the repository file layout rules.


Step 3: Timers and signals
--------------------------

TIMER ABSTRACTION COMPLEXITY:
  timerfd (Linux) and EVFILT_TIMER (macOS) have different models:
  - timerfd: Creates a file descriptor, readable when timer fires
  - EVFILT_TIMER: Timer events delivered inline with other kqueue events

  The backend interface must hide this difference. Options:
  a) On macOS, kqueue returns timer events directly in run loop
  b) On Linux, timerfd is registered like any other fd

  Suggested interface:
    add_timer(duration, callback) -> timer_id
    cancel_timer(timer_id)

  The backend translates this to timerfd or EVFILT_TIMER internally.

- Replace timerfd usage in src/neonsignal/event_loop/add_timer.c++ and
  src/neonsignal/http2_listener/start_redirect_monitor_.c++ with backend
  timers (kqueue EVFILT_TIMER on macOS, timerfd on Linux).
- Replace signalfd usage in src/neonsignal/server/run.c++ with a portable
  signal integration (kqueue EVFILT_SIGNAL or a self-pipe + signal handler).
- Ensure EventLoop callback signatures remain consistent across platforms.


Step 4: Socket accept and flags
-------------------------------

- Add portable helpers in include/neonsignal/socket_utils.h++ and
  src/neonsignal/socket_utils/*.c++ for:
  - accept_nonblocking(): accept() + set_nonblocking() + set_cloexec()
  - set_nonblocking(): fcntl(fd, F_SETFL, O_NONBLOCK)
  - set_cloexec(): fcntl(fd, F_SETFD, FD_CLOEXEC)
- Update accept4 usage in:
  src/neonsignal/http2_listener/handle_accept_.c++,
  src/neonsignal/redirect_service/handle_accept_.c++
- Update socket creation flag usage in:
  src/neonsignal/http2_listener/helper/make_listen_socket.c++,
  src/neonsignal/redirect_service/setup_listener_.c++


Step 5: SIGPIPE and thread naming
---------------------------------

Use inline #ifdef for these small differences:

MSG_NOSIGNAL (send flag):
  #ifdef __APPLE__
    // Set SO_NOSIGPIPE on socket at creation time instead
    int flags = 0;
  #else
    int flags = MSG_NOSIGNAL;
  #endif

  Update: src/neonsignal/redirect_service/handle_io_.c++
  Also set SO_NOSIGPIPE in socket_utils for macOS.

pthread_setname_np (thread naming):
  #ifdef __APPLE__
    pthread_setname_np(name);  // Sets current thread only
  #else
    pthread_setname_np(pthread_self(), name);
  #endif

  Update: src/redirect_main.c++, src/neonsignal/thread_pool.c++,
          src/neonsignal/logging.c++


Step 6: Meson build changes
---------------------------

Source file selection:
  if host_machine.system() == 'linux'
    event_loop_sources = files(
      'event_loop/linux/backend.c++',
      'event_loop/linux/add_fd.c++',
      ...
    )
  elif host_machine.system() == 'darwin'
    event_loop_sources = files(
      'event_loop/darwin/backend.c++',
      'event_loop/darwin/add_fd.c++',
      ...
    )
  endif

OpenSSL and dependency discovery on macOS:
  Homebrew installs to different paths based on architecture:
  - Apple Silicon: /opt/homebrew/opt/openssl@3
  - Intel Mac: /usr/local/opt/openssl@3

  Add to meson.build or document for users:
    if host_machine.system() == 'darwin'
      # User must set PKG_CONFIG_PATH before running meson setup
      # export PKG_CONFIG_PATH="/opt/homebrew/opt/openssl@3/lib/pkgconfig:$PKG_CONFIG_PATH"
    endif

  Alternatively, use meson's dependency() with fallback paths:
    openssl_dep = dependency('openssl', version: '>=3.0',
      required: true,
      include_type: 'system')

macOS dependency installation (document in README):
  # Apple Silicon
  brew install openssl@3 nghttp2 libmdbx pkg-config meson

  # Before building
  export PKG_CONFIG_PATH="/opt/homebrew/opt/openssl@3/lib/pkgconfig"
  export PKG_CONFIG_PATH="/opt/homebrew/opt/libmdbx/lib/pkgconfig:$PKG_CONFIG_PATH"
  export PKG_CONFIG_PATH="/opt/homebrew/opt/nghttp2/lib/pkgconfig:$PKG_CONFIG_PATH"

  meson setup build
  meson compile -C build


Step 7: Verification and testing
--------------------------------

Toolchain requirements:
- Xcode 15+ / clang 16+ with libc++ and std::format support
- Alternatively: Homebrew llvm@18 if Xcode clang lacks std::format

Build verification:
  meson setup build
  meson compile -C build

Smoke tests (manual):
  # Start server
  ./build/src/neonsignal &

  # Basic HTTP/2 connectivity
  curl -k --http2 https://localhost:9443/

  # Verify SSE streams work (tests timer/event delivery)
  curl -k --http2 -N https://localhost:9443/api/events

  # Load test (verify edge-trigger handling under concurrency)
  h2load -n 1000 -c 10 https://localhost:9443/

  # Test file upload (exercises chunked read handling)
  curl -k --http2 -X POST -F "file=@largefile.bin" https://localhost:9443/api/incoming_data

Platform-specific edge cases to test:
- [ ] SIGINT/SIGTERM handling (graceful shutdown)
- [ ] Timer accuracy (SSE batching at ~16ms)
- [ ] Large request bodies arriving in chunks
- [ ] Connection cleanup on client disconnect
- [ ] Thread pool task execution
- [ ] OpenSSL TLS handshake completion

Known differences to document:
- Socket buffer sizes may differ (check SO_SNDBUF/SO_RCVBUF defaults)
- Maximum open file descriptors (ulimit -n) may need adjustment
- kqueue has different error reporting than epoll for some edge cases


Future Considerations
---------------------

BSD support:
  kqueue implementation would also work on FreeBSD/OpenBSD with minimal
  changes. Consider naming the backend "bsd" or "kqueue" instead of "darwin"
  if BSD support is desired later.

Windows support:
  Would require IOCP backend - significantly more work. Not recommended
  unless there's a specific use case.

CI/CD:
  Consider GitHub Actions matrix build for linux + macos to catch
  portability regressions early.
