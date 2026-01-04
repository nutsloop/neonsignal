# Getting Started

This guide will help you build and run NeonSignal from source.

## Prerequisites

### System Requirements

- **OS:** Linux (tested on Oracle Linux 10, RHEL 9, Fedora 40+)
- **Architecture:** x86_64 or ARM64
- **Memory:** 2GB RAM minimum (4GB recommended for development)

### Dependencies

Install the required packages:

::::{tab-set}

:::{tab-item} Oracle Linux / RHEL
```bash
sudo dnf install -y \
    gcc-c++ \
    meson \
    ninja-build \
    openssl-devel \
    libnghttp2-devel
```
:::

:::{tab-item} Fedora
```bash
sudo dnf install -y \
    gcc-c++ \
    meson \
    ninja-build \
    openssl-devel \
    libnghttp2-devel
```
:::

:::{tab-item} Ubuntu / Debian
```bash
sudo apt install -y \
    g++ \
    meson \
    ninja-build \
    libssl-dev \
    libnghttp2-dev
```
:::

::::

### Compiler Requirements

NeonSignal requires a C++23 compatible compiler:

- **GCC 14+** (recommended)
- **Clang 18+**

## Building from Source

### Clone the Repository

```bash
git clone https://github.com/simonedelpopolo/neonsignal.git
cd neonsignal
```

### Configure and Build

```bash
# Configure with Meson (debug build)
meson setup build

# Or for release build with optimizations
meson setup build -Dbuildtype=release

# Compile
meson compile -C build
```

The binaries will be created in `build/src/`:
- `neonsignal` — The main HTTP/2 server
- `neonsignal_redirect` — HTTP→HTTPS redirect service

### Run the Tests

```bash
meson test -C build
```

## Running NeonSignal

### Generate Development Certificates

For local development, generate self-signed certificates:

```bash
./scripts/mkcert_local.sh
```

This creates certificates in `certs/` for local `.host` domains.

### Start the Server

```bash
# Run directly from build directory
./build/src/neonsignal
```

By default, the server:
- Listens on port **9443** (HTTPS)
- Serves files from `public/`
- Uses certificates from `certs/`

### Configuration via Environment Variables

| Variable               | Default   | Description           |
|------------------------|-----------|-----------------------|
| `NEONSIGNAL_PORT`      | `9443`    | HTTPS listen port     |
| `NEONSIGNAL_HOST`      | `0.0.0.0` | Bind address          |
| `NEONSIGNAL_INSTANCES` | `3`       | Worker thread count   |
| `CERTS_ROOT`           | `certs`   | Certificate directory |
| `PUBLIC_ROOT`          | `public`  | Document root         |

Example:

```bash
NEONSIGNAL_PORT=8443 NEONSIGNAL_INSTANCES=4 ./build/src/neonsignal
```

### Start the Redirect Service

The redirect service handles HTTP→HTTPS redirects and ACME challenges:

```bash
./build/src/neonsignal_redirect
```

| Variable               | Default          | Description              |
|------------------------|------------------|--------------------------|
| `REDIRECT_PORT`        | `9090`           | HTTP listen port         |
| `REDIRECT_TARGET_PORT` | `443`            | HTTPS redirect target    |
| `REDIRECT_INSTANCES`   | `1`              | Worker count             |
| `ACME_WEBROOT`         | `acme-challenge` | ACME challenge directory |

## Quick Test

Once the server is running, test it with curl:

```bash
# HTTP/2 request (skip cert verification for self-signed)
curl -k --http2 https://localhost:9443/

# Check HTTP/2 is working
curl -k -I --http2 https://localhost:9443/ 2>&1 | grep HTTP
# Should show: HTTP/2 200
```

## Production Deployment

For production deployment with systemd services and Let's Encrypt certificates, see the [Deployment Guide](part4-operations/deployment).

## Directory Structure

```
neonsignal/
├── build/              # Build output (generated)
├── certs/              # TLS certificates
│   ├── _default/       # Fallback certificate
│   └── <domain>/       # Per-domain certificates
├── public/             # Static files (document root)
│   ├── _default/       # Fallback content
│   └── <domain>/       # Per-domain content
├── src/                # C++ source code
├── include/            # Header files
├── scripts/            # Build and deployment scripts
├── systemd/            # Service unit files
└── docs/               # This documentation
```

## Next Steps

- Learn about [Virtual Hosting](part2-architecture/virtual-hosting) for multi-domain setups
- Set up [Let's Encrypt certificates](part4-operations/deployment) for production
- Explore [SSE streaming](part3-features/sse) for real-time features
