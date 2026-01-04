# NeonSignal

A high-performance HTTP/2 server written in modern C++23, featuring a custom JSX runtime, TLS 1.3, virtual hosting, WebAuthn authentication, and Server-Sent Events broadcasting.

## Features

### Core Server
- **Pure C++23 Implementation** - Leverages coroutines, concepts, ranges, `std::expected`, and `std::format`
- **HTTP/2 Native** - Full HTTP/2 support with HPACK header compression via nghttp2
- **TLS 1.3** - Secure-by-default with OpenSSL 3.0+, ALPN negotiation for `h2`
- **epoll Event Loop** - Non-blocking I/O with efficient edge-triggered event handling
- **Thread Pool** - Configurable worker threads for CPU-bound tasks and TLS handshake offloading
- **Graceful Shutdown** - Connection draining with configurable timeout

### Performance Optimizations
- **Static File Cache** - In-memory LRU cache (default 50MB) with ETag generation
- **Session Cache** - Avoid re-parsing cookies on every request
- **SSE Broadcaster** - Batched Server-Sent Events with pre-encoded HTTP/2 DATA frames (~60fps batching)
- **Connection Manager** - Resource limits and DoS protection (10k max connections, 100 streams per connection)
- **Write Backpressure** - Prevents memory exhaustion from slow clients

### Virtual Hosting
- **Directory-Based VHosts** - Drop domain directories in `public/` for automatic routing
- **Wildcard Domains** - Support for `*.example.com` style configurations
- **NeonJSX SPA Routing** - Per-vhost client-side routing configuration

### Authentication & Storage
- **WebAuthn/Passkeys** - Passwordless authentication with challenge-response flow
- **MDBX Database** - Embedded key-value store for users, sessions, and application data
- **Codex System** - Document storage with metadata, images, and command execution tracking

### Custom JSX Runtime (NeonJSX)
- **Lightweight VDOM** - Custom `h()` and `Fragment` implementation
- **Zero Dependencies** - No React required, pure TypeScript/JavaScript
- **esbuild Bundling** - Fast builds with tree-shaking and minification

## Prerequisites

### System Dependencies

**Fedora/RHEL/CentOS:**
```bash
sudo dnf install -y gcc-c++ meson ninja-build openssl-devel libnghttp2-devel cmake
```

**Ubuntu/Debian:**
```bash
sudo apt install -y g++ meson ninja-build libssl-dev libnghttp2-dev cmake
```

**macOS:**
```bash
brew install meson ninja openssl@3 nghttp2 cmake
```

### Build Requirements
- GCC 13+ or Clang 17+ (C++23 support required)
- Meson 1.0+
- OpenSSL 3.0+
- nghttp2
- libmdbx (built from subproject if not found)

### Frontend Requirements
- Node.js 18+
- npm 9+

## Installation

### Clone the Repository
```bash
git clone https://github.com/nutsloop/neonsignal.git
cd neonsignal
```

### Install Node Dependencies
```bash
npm install
```

### Configure and Build the Server
```bash
# Configure (one-time)
meson setup build

# Build
meson compile -C build
```

The server binary is located at `build/src/neonsignal`.

### Build Frontend Applications
```bash
# Build the NeonJSX runtime
npm run build:neonjsx

# Build vhost applications
npm run build:neonsignal   # neonsignal.nutsloop.host
npm run build:nutsloop     # nutsloop.host
npm run build:simonedelpopolo  # simonedelpopolo.host
```

### Generate TLS Certificates

For local development:
```bash
./scripts/mkcert_local.sh
```

For production (Let's Encrypt):
```bash
./scripts/letsencrypt.sh
```

## Usage

### Running the Server

```bash
# Run with defaults (0.0.0.0:9443)
./build/src/neonsignal

# Or install system-wide
sudo meson install -C build
neonsignal
```

### Environment Configuration

Configure the server via environment variables:

| Variable | Default | Description |
|----------|---------|-------------|
| `NEONSIGNAL_HOST` | `0.0.0.0` | Bind address |
| `NEONSIGNAL_PORT` | `9443` | HTTPS port |
| `NEONSIGNAL_THREADS` | `3` | Worker thread count |
| `NEONSIGNAL_RP_ID` | `neonsignal.nutsloop.host` | WebAuthn relying party ID |
| `NEONSIGNAL_ORIGIN` | `https://neonsignal.nutsloop.host` | WebAuthn origin |

### systemd Service

A systemd unit file is provided for production deployments:

```bash
sudo cp systemd/neonsignal.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now neonsignal
```

### Running the HTTP to HTTPS Redirect Service

```bash
./build/src/neonsignal_redirect
```

### Testing the Server

```bash
# HTTP/2 request with curl
curl -k --http2 https://localhost:9443/

# Load testing with h2load
h2load -n 1000 -c 10 -m 10 https://localhost:9443/index.html
```

## Project Structure

```
neonsignal/
├── include/neonsignal/     # Public headers
│   ├── neonsignal.h++      # Server configuration and entry point
│   ├── event_loop.h++      # epoll-based event loop
│   ├── http2_listener.h++  # HTTP/2 connection handling
│   ├── thread_pool.h++     # Task execution pool
│   ├── vhost.h++           # Virtual host resolution
│   ├── database.h++        # MDBX wrapper
│   ├── webauthn.h++        # Passkey authentication
│   ├── sse_broadcaster.h++ # Server-Sent Events
│   ├── static_cache.h++    # File caching
│   ├── session_cache.h++   # Session validation cache
│   └── connection_manager.h++ # Connection limits
├── src/
│   ├── main.c++            # Entry point
│   └── neonsignal/         # Implementation files
│       ├── event_loop/     # Event loop methods
│       ├── http2_listener/ # HTTP/2 frame handling
│       ├── api_handler/    # REST API endpoints
│       ├── server/         # Server lifecycle
│       └── ...
├── neonjsx/                # JSX runtime
│   ├── runtime.ts          # h(), Fragment, render()
│   └── jsx.d.ts            # TypeScript declarations
├── www/                    # Virtual hosts
│   ├── neonsignaljsx/      # neonsignal.nutsloop.host
│   ├── nutsloopjsx/        # nutsloop.host
│   ├── simonedelpopolo/    # simonedelpopolo.host
│   ├── book/               # Documentation site
│   └── _default/           # Default fallback
├── public/                 # Built static assets
├── certs/                  # TLS certificates
├── data/                   # MDBX database files
├── scripts/                # Build and deployment scripts
│   ├── sphinx/             # Documentation generation
│   ├── letsencrypt.sh      # Certificate automation
│   └── rsync_deploy.sh     # Deployment script
├── systemd/                # Service files
├── benchmark/              # Performance test results
├── meson.build             # C++ build configuration
├── package.json            # Node.js dependencies
└── tsconfig.json           # TypeScript configuration
```

## Configuration

### Server Configuration (ServerConfig struct)

```cpp
struct ServerConfig {
  std::string host = "0.0.0.0";
  std::uint16_t port = 9443;
  std::string certs_root = "certs";
  std::string public_root = "public";
  std::string rp_id = "neonsignal.nutsloop.host";
  std::string origin = "https://neonsignal.nutsloop.host";
  std::string credentials_path = "config/credentials.json";
  std::string db_path = "data/neonsignal.mdb";
};
```

### Connection Limits (ConnectionManager)

| Limit | Value | Description |
|-------|-------|-------------|
| `MAX_CONNECTIONS` | 10,000 | Maximum concurrent connections |
| `MAX_STREAMS_PER_CONNECTION` | 100 | HTTP/2 multiplexed streams |
| `MAX_WRITE_BUFFER_BYTES` | 256 KB | Per-connection write buffer |
| `MAX_READ_BUFFER_BYTES` | 1 MB | Per-connection read buffer |
| `MAX_UPLOAD_SIZE_BYTES` | 100 MB | Maximum file upload size |
| `HANDSHAKE_TIMEOUT` | 10s | TLS handshake deadline |
| `IDLE_TIMEOUT` | 300s | Connection idle timeout |

### Static File Cache

- Default size: 50 MB
- Pre-loads files under 100 KB at startup
- LRU eviction when full
- Automatic ETag generation

### NeonJSX Configuration

Create a `.neonjsx` file in your vhost public directory with one route per line:

```
/
/about
/dashboard
/blog/*
/docs/*
```

## Building Documentation

Sphinx documentation can be generated using the provided scripts:

```bash
# Setup Sphinx environment
./scripts/sphinx/setup.sh

# Generate documentation
./scripts/sphinx/build.sh

# Or run all steps
./scripts/sphinx/all.sh
```

## Performance

Recent benchmark results (h2load, single server):

| Metric | Value |
|--------|-------|
| Throughput | ~8,700 req/s |
| Mean latency | 11.35 ms |
| Min latency | 0.2 ms |
| Max latency | 122 ms |
| Error rate | 0% |

Benchmarks performed with `h2load` against `/index.html` endpoint.

## API Endpoints

### Server Statistics
- `GET /api/stats` - Server metrics (connections, served files, page views)

### Server-Sent Events
- `GET /api/events` - General event stream
- `GET /api/cpu` - CPU usage metrics stream
- `GET /api/memory` - Memory usage metrics stream
- `GET /api/redirect` - Redirect service status stream

### WebAuthn Authentication
- `GET /api/auth/login/options` - Get login challenge
- `POST /api/auth/login/finish` - Complete login
- `POST /api/auth/user/register` - Create user account (returns verification token)
- `POST /api/auth/user/verify` - Verify user and create pre-WebAuthn session
- `GET /api/auth/user/enroll` - Get enrollment challenge (pre-WebAuthn session required)
- `POST /api/auth/user/enroll` - Complete enrollment (pre-WebAuthn session required)
- `GET /api/auth/user/check` - Validate session

### Codex (Document Storage)
- `GET /api/codex/list` - List stored documents
- `GET /api/codex/item/<id>` - Get document metadata
- `GET /api/codex/image/<id>` - Get document image
- `POST /api/codex/brief` - Create new document

## Development

### Code Style
- C++ uses 2-space indentation
- Private class members suffixed with `_`
- Files use `snake_case.c++`/`.h++` naming
- TypeScript/JSX components use PascalCase

### Adding a New VHost

1. Create a directory in `www/` named after your domain
2. Add a `build.sh` script for asset compilation
3. Add static assets to a `static/` subdirectory
4. Optionally add `.neonjsx` file for SPA routing (one route per line)

### Testing

```bash
# Manual HTTP/2 testing
curl -k --http2 https://localhost:9443/

# Verbose with headers
curl -k --http2 -v https://localhost:9443/

# WebSocket/SSE testing
curl -k --http2 -N https://localhost:9443/api/events
```

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](./LICENSE) for details.
