# Repository Guidelines

## Policy
- WARNING: Never run `sudo` unless the user explicitly requests it; even then, always ask for confirmation before running any `sudo` command.

## Project Structure & Module Organization
- Core server lives in `src/` with HTTP/2 and TLS pieces under `src/neonsignal/` (event loop, thread pool, router, HPACK via nghttp2, HTTP/2 listener). Public headers mirror this in `include/neonsignal/`.
- Entry point is `src/main.c++`; the Meson target outputs `build/src/neonsignal`.
- Front-end runtime lives in `neonjsx/` (shared `runtime.ts` + `jsx.d.ts`). App sources live in `neonsignaljsx/` and `nutsloopjsx/`. Bundled assets land in `public/` (`app.js`, `app.js.map`, static HTML/CSS/JS, certs live in `certs/`).
- Build files: root `meson.build` plus `package.json`, `tsconfig.json`; node modules are vendored in `node_modules/`.

## Build, Test, and Development Commands
- Configure & build C++: `meson setup build` (once), then `meson compile -C build`. Run with `./build/src/neonsignal`.
- Front-end bundle (neonsignal app): `npm run neonsignal.nutsloop.host` (transpiles `neonsignaljsx/` to `build/neonsignaljsx/`, bundles to `public/neonsignal.nutsloop.host/app.js`).
- Front-end bundle (nutsloop app): `npm run build:nutsloop` (transpiles `nutsloopjsx/`, bundles to `public/nutsloop.host/app.js`).
- Clean front-end artifacts (neonsignal app): `npm run clean:neonsignal`.
- No formal test suite yet; favor targeted manual checks (curl, browser with HTTP/2 ALPN, `h2load` for load) and add tests alongside new modules when practical.

## Coding Style & Naming Conventions
- Use C++23 features; prefer `std::format` for logging/string assembly over manual concatenation/streams. C++ uses 2-space indentation, RAII, and standard library utilities; prefer `std::unique_ptr`/`std::shared_ptr` as appropriate. Keep headers in `include/neonsignal/` paired with `src/neonsignal/` implementations.
- Favor self-documenting names (`event_loop`, `http2_listener`, `router`). Keep files lowercase with `snake_case.c++`/`.h++`.
- private member of classes always suffixed with a low-dash `_`
- the implementation of methods of a class should be stored into a file named after the method under a directory named after the class. constructors are implemented in the c++ file named after the class.
- TypeScript/JSX uses the custom JSX factory `h`/`Fragment`; keep components in PascalCase files under `neonsignaljsx/` or `nutsloopjsx/` as appropriate.
- Run formatters if added; otherwise match existing style and keep comments minimal and purposeful.

## Testing Guidelines
- Add lightweight unit/integration tests near new code (e.g., `tests/` or module-adjacent) when introducing behavior that can regress (routing, HPACK handling, SSE streams).
- Name tests after the behavior under test; prefer deterministic cases that do not require network.
- For manual checks, document the command you used (e.g., `curl -k --http2 https://10.0.0.10:9443/`).

## Commit & Pull Request Guidelines
- Always wait to commit when requested, never do for every change
- Use concise, descriptive commits; conventional prefixes seen here (`feat:`, `fix:`, `chore:`). Example: `feat: add SSE memory stream`.
- Always follow the subject with a blank line and a short body explaining the change (what/why); keep commits scoped and avoid committing build artifacts unless required.
- PRs should summarize changes, mention affected modules, list verification steps (builds, curl/browser checks, load tests), and include screenshots/GIFs for UI updates.
- Use detailed description for every commit
- try to logical commit file related by mean.

## Security & Configuration Tips
- Certificates are self-signed in `certs/`; rotate/regenerate for production and guard private keys.
- The server defaults to HTTP/2 over TLS; ensure ALPN `h2` is negotiated by clients. Avoid running with weak ciphers or without TLS in production.
