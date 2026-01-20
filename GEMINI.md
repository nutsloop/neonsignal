# Project: NeonSignal

## Project Overview

**NeonSignal** is a high-performance, C++23-based HTTP/2 server designed for real-time applications. It features a monolithic repository structure that integrates a custom JSX-based frontend, Sphinx for documentation, and AI-powered content generation.

The server is built from the ground up to be compliant with RFC 9113 (HTTP/2), and it leverages modern C++23 features like coroutines, concepts, and ranges. It includes support for TLS 1.3+, SNI-based virtual hosting, and uses `libmdbx` for embedded database storage. The frontend is powered by a custom JSX runtime, independent of React.

The project is comprised of two main executables: `neonsignal`, the main HTTP/2 server, and `neonsignal_redirect`, a simple HTTP to HTTPS redirector.

## Building and Running

### C++ Backend

The C++ backend is built using the **Meson** build system.

**Dependencies:**
- OpenSSL (>= 3.0.0)
- libnghttp2
- libmdbx

**Build Commands:**
```bash
# Configure the build (run once)
meson setup build

# Build the project
meson compile -C build
```

The compiled binaries will be located in the `build/src/` directory.

**Running the Server:**
```bash
# Run the main HTTP/2 server
./build/src/neonsignal spin [options]

# Run the HTTP to HTTPS redirector
./build/src/neonsignal_redirect spin [options]
```
*Note: The `spin` command is required unless running with the `--systemd` flag.*

### Frontend

The frontend is built using **Node.js**, **TypeScript**, and **esbuild**.

**Dependencies:**
- Node.js & npm
- TypeScript
- esbuild
- Babel

**Build Commands:**
```bash
# Install dependencies
npm install

# Build the frontend applications
npm run build:neonsignal
npm run build:nutsloop
# ... and so on for other applications
```
The build scripts in `package.json` handle the transpilation and bundling of the various frontend applications.

## Development Conventions

### C++

- **Standard:** C++23
- **Indentation:** 2 spaces
- **File Naming:** `snake_case.c++` for source files and `snake_case.h++` for headers.
- **Private Members:** Private class members are suffixed with an underscore (e.g., `my_variable_`).
- **Source Structure:** Header files in `include/neonsignal/` are paired with implementation files in `src/neonsignal/`. Method implementations are in files named after the method, under a directory named after the class.

### TypeScript/JSX

- **JSX Runtime:** A custom JSX factory (`h`/`Fragment`) is used, not React.
- **Component Naming:** Components are in `PascalCase` files.
- **Linting:** ESLint is used with the TypeScript parser.

### Commits

- **Conventional Commits:** Commit messages should follow the conventional commits specification (e.g., `feat:`, `fix:`, `chore:`).
- **Scope:** Commits should be scoped to a single logical change.

This document provides a high-level overview of the NeonSignal project. For more detailed information, please refer to the `README.md` and `CLAUDE.md` files.
