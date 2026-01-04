# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

NeonSignal is a C++23 HTTP/2 server with TLS/SSL, featuring a custom JSX frontend runtime and Sphinx documentation. The server uses epoll-based event handling, nghttp2 for HTTP/2 frames, and libmdbx for embedded storage.

## Build Commands

### C++ Backend (Meson)
```bash
meson setup build                    # First-time setup
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
npm run lint                         # Lint TypeScript
npm run lint-fix                     # Auto-fix lint issues
```

### Sphinx Documentation
```bash
./scripts/sphinx/all.sh              # Quick rebuild
./scripts/sphinx/all.sh --clean      # Clean rebuild (keeps venv)
./scripts/sphinx/all.sh --fresh      # Full reset (removes venv)
./scripts/sphinx/all.sh --no-dynamics # Skip dynamic content
```

### Release Build
```bash
./scripts/build-release.sh           # Complete release build
```

## Build Workflow (IMPORTANT)

**ALWAYS ask before building to save tokens.** Build processes consume significant tokens and should only be run when explicitly requested by the user or when absolutely necessary.

- ❌ Don't automatically run `meson compile`, `npm run build:*`, or other build commands after making changes
- ✅ Do make code changes and wait for user to request build
- ✅ Do ask "Should I build this?" when uncertain
- Exception: Only build without asking if the user explicitly requests it (e.g., "build and test this")

## Architecture

### Directory Structure
```
src/neonsignal/          # C++ implementations (event_loop, http2_listener, router, etc.)
include/neonsignal/      # C++ headers (mirror src/ structure)
www/                     # Frontend sources
  ├── book/              # Sphinx documentation source
  ├── neonsignaljsx/     # NeonSignal JSX app
  └── nutsloopjsx/       # NutsLoop JSX app
neonjsx/                 # Shared JSX runtime (h(), Fragment, render())
build/                   # All build artifacts (C++, transpiled JS, Sphinx HTML)
public/                  # Deployed web assets (served by server)
scripts/                 # Build and deployment scripts
  ├── global_variables.sh    # Centralized paths
  ├── sphinx/                # Sphinx pipeline scripts
  └── lib/logging.sh         # Shared logging functions
```

### Build Pipelines

**C++ Backend**: `meson compile` → `build/src/neonsignal`

**Frontend**: TSX → Babel transpile → `build/*/` → esbuild bundle → `public/*/app.js`

**Sphinx Docs**: `www/book/source/` → `build/www/book/` → `public/neonsignal.nutsloop.net/book/`

### Two Executables
- `neonsignal` - Main HTTP/2 server (port 9443)
- `neonsignal_redirect` - HTTP→HTTPS redirector

## Code Style

### C++
- C++23 standard, 2-space indentation
- Private members suffixed with `_`
- Method implementations in subdirectory named after class (e.g., `http2_listener/handle_io_.c++`)
- Use `std::format` for logging/string assembly
- Files: `snake_case.c++`/`.h++`

### TypeScript/JSX
- Custom JSX factory: `h`/`Fragment` (not React)
- PascalCase for component files
- ESLint with TypeScript parser

## Testing

No formal test suite. Manual testing approaches:
```bash
curl -k --http2 https://localhost:9443/      # HTTP/2 check
h2load -n 1000 -c 10 https://localhost:9443/ # Load testing
```

## Key Configuration

- Server config: `include/neonsignal/neonsignal.h++` (ServerConfig struct)
- Routes: `include/neonsignal/routes.h++`
- Babel JSX: `.babelrc`
- TypeScript: `tsconfig.json`
- C++ format: `.clang-format`
- Global paths: `scripts/global_variables.sh`

## Commit Guidelines

- Wait for explicit commit request
- Use conventional prefixes: `feat:`, `fix:`, `chore:`
- Include description body explaining what/why
- Group logically related files in single commits
