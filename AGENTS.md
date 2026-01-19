# Repository Guidelines

## Policy
- WARNING: Never run `sudo` unless the user explicitly requests it; even then, always ask for confirmation before running any `sudo` command.

### use of Emoticons
**IMPORTANT:** Avoid emoji in docs/logs/source everywhere; use plain Unicode symbols.
Recommended Unicode symbols:
- ✓ check
- ✗ cross
- • bullet
- ‣ bullet (alt)
- ◦ bullet (alt)
- → arrow right
- ← arrow left
- ↑ arrow up
- ↓ arrow down
- ↳ return/branch
- ⇢ arrow right (alt)
- ⇒ implies
- ⇐ implies (rev)
- ⇔ iff
- ↔ bidirectional
- ⟶ long arrow
- ⟵ long arrow (rev)
- ⟷ long bidirectional
- ∎ end marker
- □ empty square
- ■ filled square
- ◆ filled diamond
- ◇ empty diamond
- ○ empty circle
- ● filled circle
- ▸ triangle right
- ▾ triangle down
- ▲ triangle up
- ▼ triangle down (alt)
- ⌁ wave
- ⋯ ellipsis (midline)
- ≈ approximately
- ≠ not equal
- ≤ less-than-or-equal
- ≥ greater-than-or-equal
- ± plus/minus
- ∞ infinity
- ∅ empty set
- ∈ member of
- ∉ not member of
- ⊂ subset
- ⊃ superset
- ¬ logical not

Expressive symbols:
- ✦ star
- ✧ star (outline)
- ✶ star (alt)
- ✷ star (alt)
- ✸ star (alt)
- ✹ star (alt)
- ✺ star (alt)
- ✻ star (alt)
- ✼ star (alt)
- ✽ star (alt)
- ✾ star (alt)
- ✿ floral
- ❀ floral (alt)
- ❁ floral (alt)
- ❂ sun
- ❃ floral (alt)
- ❄ snowflake
- ❅ snowflake (alt)
- ❆ snowflake (alt)
- ❇ sparkle
- ❈ sparkle (alt)
- ❉ sparkle (alt)
- ❊ sparkle (alt)
- ❋ sparkle (alt)
- ❖ ornamental diamond
- ✱ asterisk star
- ✲ asterisk star (alt)
- ✳ asterisk star (alt)
- ❘ light vertical
- ❘̸ barred vertical
- ❙ heavy vertical
- ❚ heavy vertical (alt)
- ❛ single quote (ornate)
- ❜ single quote (ornate)
- ❝ double quote (ornate)
- ❞ double quote (ornate)
- ❦ floral heart
- ❧ rotated floral heart
- ❥ heart (ornate)
- ❍ ring
- ⊕ circled plus
- ⊗ circled times
- ⊙ circled dot
- ⊚ circled dot (alt)
- ⊛ circled star
- ⊜ circled equal
- ♠ spade (ace)
- ♥ heart (ace)
- ♦ diamond (ace)
- ♣ club (ace)
- ✦ future...

## Project Overview

NeonSignal is a C++23 HTTP/2 server with TLS/SSL, featuring a custom JSX frontend runtime and Sphinx documentation. The server uses epoll-based event handling, nghttp2 for HTTP/2 frames, and libmdbx for embedded storage.

## Dependencies

- C++/backend: OpenSSL, nghttp2 (HPACK), libmdbx.
- Build tooling: Meson.
- Frontend toolchain: Node.js + npm, Babel, TypeScript, esbuild, ESLint.
- Docs: Python 3, Sphinx, myst-parser, sphinx-copybutton, sphinx-design, sphinx-synthwave-theme.

## Build Commands

### C++ Backend (Meson)
```bash
meson setup build                    # First-time setup
meson setup --reconfigure build      # Reconfigure existing build dir
meson setup --wipe build             # Wipe and reconfigure from scratch
meson compile -C build               # Build
./build/src/neonsignal               # Run main server
./build/src/neonsignal_redirect      # Run HTTP→HTTPS redirector
```

### Frontend (TypeScript/JSX)
```bash
npm install                          # Install dependencies
npm run build:neonjsx                # Build shared JSX runtime
npm run build:neonsignal             # Build neonsignal app
npm run build:nutsloop               # Build nutsloop app
npm run build:simonedelpopolo        # Build simonedelpopolo app
npm run build:_default               # Build default app
npm run clean:neonjsx                # Clean shared JSX runtime
npm run clean:neonsignal             # Clean neonsignal app
npm run clean:nutsloop               # Clean nutsloop app
npm run clean:simonedelpopolo        # Clean simonedelpopolo app
npm run clean:_default               # Clean default app
npm run all:neonjsx                  # Clean + build shared JSX runtime
npm run all:neonsignal               # Clean + build neonsignal app
npm run all:nutsloop                 # Clean + build nutsloop app
npm run all:simonedelpopolo          # Clean + build simonedelpopolo app
npm run all:_default                 # Clean + build default app
npm run lint                         # Lint TypeScript
npm run lint-fix                     # Auto-fix lint issues
```

### Release Build
```bash
./scripts/build/only-build.sh        # Release build (no wipe; builds backend only)
./scripts/build/all.sh               # Full release build + deploy (systemd, certs, Sphinx; uses sudo)
./scripts/build/all.sh --local       # Local build + deploy (keeps .host, local certs; uses sudo)
./scripts/build/all.sh --delete-build # Wipe build/ before configure
./scripts/build/all.sh --reset-db    # Wipe data/ before start
./scripts/build/install.sh           # Install binaries to /usr/local/bin (SELinux; uses sudo)
```

## Build Workflow (IMPORTANT)

**ALWAYS ask before building to save tokens.** Build processes consume significant tokens and should only be run when explicitly requested by the user or when absolutely necessary.

- ✗ Don't automatically run `meson compile`, `npm run build:*`, or other build commands after making changes
- ✓ Do make code changes and wait for user to request build
- ✓ Do ask "Should I build this?" when uncertain
- Exception: Only build without asking if the user explicitly requests it (e.g., "build and test this")

## Architecture

### Directory Structure
```
src/neonsignal/          # C++ implementations (event_loop, http2_listener, router, etc.)
include/neonsignal/      # C++ headers (mirror src/ structure)
www/                     # Frontend sources
  ├── neonsignaljsx/     # neonsignal JSX app
  ├── nutsloopjsx/       # nutsloop JSX app
  ├── simonedelpopolo/   # simonedelpopolo site
  └── _default/          # default vhost site
neonjsx/                 # Shared JSX runtime (h(), Fragment, render())
build/                   # All build artifacts (C++, transpiled JS, Sphinx HTML)
public/                  # Deployed web assets (served by server)
benchmark/               # Benchmark reports (source)
plans/                   # AI collaboration plans
certs/                   # TLS certificates (self-signed)
systemd/                 # systemd service units
scripts/                 # Build and deployment scripts
  ├── global_variables.sh    # Centralized paths
  ├── build/                 # Release/deploy scripts
  ├── vhost/                 # Virtual host helpers
  └── lib/logging.sh         # Shared logging functions
```

### Build Pipelines

**C++ Backend**: `meson compile` → `build/src/neonsignal`

**Frontend**: TSX → Babel transpile → `build/*/` → esbuild bundle → `public/*/app.js`

### Two Executables
- `neonsignal` - Main HTTP/2 server (port 9443)
- `neonsignal_redirect` - HTTP→HTTPS redirector

## Code Style

### C++
- C++23 standard; prefer `std::format` for logging/string assembly.
- 2-space indentation, RAII, and standard library utilities.
- Prefer `std::unique_ptr`/`std::shared_ptr` as appropriate.
- Keep headers in `include/neonsignal/` paired with `src/neonsignal/` implementations.
- Favor self-documenting names (`event_loop`, `http2_listener`, `router`).
- Files: lowercase `snake_case.c++`/`.h++`.
- Private members must be suffixed with `_`.
- Method implementations live in a file named after the method, under a directory named after the class.
- Constructors are implemented in the `.c++` file named after the class.
- Keep comments minimal and purposeful; run formatters if added, otherwise match existing style.

### TypeScript/JSX
- Custom JSX factory: `h`/`Fragment` (not React).
- Keep components in PascalCase files under `neonsignaljsx/` or `nutsloopjsx/`.
- ESLint with the TypeScript parser.
- Keep comments minimal and purposeful; match existing style.

## Key Configuration

- Server config: `include/neonsignal/neonsignal.h++` (ServerConfig struct)
- Routes: `include/neonsignal/routes.h++`
- Babel JSX: `.babelrc`
- TypeScript: `tsconfig.json`
- C++ format: `.clang-format`
- Global paths: `scripts/global_variables.sh`

## Commit Guidelines

- Always wait to commit until explicitly requested.
- Use concise, descriptive commits with conventional prefixes: `feat:`, `fix:`, `chore:`.
- Follow the subject with a blank line and a short body explaining what/why.
- Keep commits scoped; avoid committing build artifacts unless required.
- Use detailed descriptions for every commit.
- Group logically related files in the same commit.
- PRs should summarize changes, mention affected modules, list verification steps, and include screenshots/GIFs for UI updates.

## License

Apache License 2.0. See `LICENSE`.

## Security & Configuration Tips
- Certificates are self-signed in `certs/`; rotate/regenerate for production and guard private keys.
- The server defaults to HTTP/2 over TLS; ensure ALPN `h2` is negotiated by clients. Avoid running with weak ciphers or without TLS in production.

## Testing

No formal test suite. Manual testing approaches:
```bash
curl -k --http2 https://localhost:9443/      # HTTP/2 check
h2load -n 1000 -c 10 https://localhost:9443/ # Load testing
```

