# macOS Portability Plan

Status: Done on the OSx branch (tag `0.2.0`). The steps below reflect the
implemented changes and the OSx README build notes.

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

Status: Addressed in the OSx branch.

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

Items to verify (still recommended on macOS):
- libmdbx: Homebrew install works; ensure PKG_CONFIG_PATH includes libmdbx
  (see macOS build notes)
- /proc filesystem: Search codebase for any /proc/self/* usage (none expected)
- OpenSSL: Homebrew installs to non-standard paths (see macOS build notes)


Step 1: Event flag abstraction and header cleanup (done)
--------------------------------------------------------

- Added include/neonsignal/event_mask.h++ and wired it into
  include/neonsignal/event_loop.h++ and include/neonsignal/http2_listener.h++.
- Replaced EPOLL* usage with EventMask in:
  include/neonsignal/http2_listener.h++,
  src/neonsignal/http2_listener/handle_io_.c++,
  src/neonsignal/http2_listener/start.c++,
  src/neonsignal/redirect_service/*.c++,
  src/neonsignal/server/run.c++,
  src/neonsignal/api_handler/*.c++
- Removed <sys/epoll.h> from public headers; OS-specific headers stay in
  backend implementations.

EDGE-TRIGGER SEMANTICS WARNING:
  EPOLLET (Linux) and EV_CLEAR (macOS kqueue) are similar but not identical:
  - EPOLLET: Edge-triggered, requires draining fd until EAGAIN
  - EV_CLEAR: Resets state after event delivery, re-arms automatically

  The current codebase likely drains correctly (required for EPOLLET), so
  EV_CLEAR should work. However, verify these files handle partial reads:
  - src/neonsignal/http2_listener/handle_io_.c++
  - src/neonsignal/redirect_service/handle_io_.c++

  Test case: Large request body that arrives in multiple chunks.


Step 2: Split EventLoop backends (epoll vs kqueue) (done)
---------------------------------------------------------

- Added include/neonsignal/event_loop_backend.h++ and updated
  include/neonsignal/event_loop.h++ to use a backend instance.
- Implemented Linux backend in src/neonsignal/event_loop/linux/backend.c++
  and macOS backend in src/neonsignal/event_loop/darwin/backend.c++.
- Kept EventLoop method files in src/neonsignal/event_loop/*.c++ as
  delegators to preserve the per-method file layout.


Step 3: Timers and signals (done)
---------------------------------

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

- Replaced timerfd usage with backend timers in
  src/neonsignal/event_loop/add_timer.c++ and
  src/neonsignal/http2_listener/start_redirect_monitor_.c++.
- Implemented signal hooks in src/neonsignal/event_loop/add_signal.c++ and
  switched src/neonsignal/server/run.c++ to use them.
- Added src/neonsignal/event_loop/cancel_timer.c++ to complete the timer API.


Step 4: Socket accept and flags (done)
--------------------------------------

- Added portable helpers in include/neonsignal/socket_utils.h++ and
  src/neonsignal/socket_utils/socket_utils.c++ for:
  - accept_nonblocking(): accept() + set_nonblocking() + set_cloexec()
  - set_nonblocking(): fcntl(fd, F_SETFL, O_NONBLOCK)
  - set_cloexec(): fcntl(fd, F_SETFD, FD_CLOEXEC)
- Updated accept4 usage in:
  src/neonsignal/http2_listener/handle_accept_.c++,
  src/neonsignal/redirect_service/handle_accept_.c++
- Updated socket creation flag usage in:
  src/neonsignal/http2_listener/helper/make_listen_socket.c++,
  src/neonsignal/redirect_service/setup_listener_.c++


Step 5: SIGPIPE and thread naming (done)
----------------------------------------

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

glibc-only malloc_trim:
  Guarded malloc_trim(0) behind __GLIBC__ in
  src/neonsignal/http2_listener/handle_io_.c++.


Step 6: Meson build changes (done)
----------------------------------

Source file selection:
  Implemented backend selection and shared sources in src/meson.build
  using host_machine.system() to pick the linux/ or darwin/ backend.

macOS build notes (as documented in the OSx README):
  brew install meson ninja pkg-config cmake openssl@3 nghttp2 libmdbx

  # Apple Silicon
  export PKG_CONFIG_PATH="/opt/homebrew/opt/openssl@3/lib/pkgconfig:/opt/homebrew/opt/nghttp2/lib/pkgconfig:/opt/homebrew/opt/libmdbx/lib/pkgconfig:$PKG_CONFIG_PATH"

  # Intel macOS (use /usr/local/opt instead of /opt/homebrew/opt)
  # export PKG_CONFIG_PATH="/usr/local/opt/openssl@3/lib/pkgconfig:/usr/local/opt/nghttp2/lib/pkgconfig:/usr/local/opt/libmdbx/lib/pkgconfig:$PKG_CONFIG_PATH"

  meson setup build
  meson compile -C build


Step 7: Verification and testing (manual checklist)
---------------------------------------------------

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
