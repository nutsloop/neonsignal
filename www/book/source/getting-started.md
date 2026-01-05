# Getting Started

This guide will help you build and run NeonSignal from source.

## Monolithic Repository Structure

NeonSignal is a **monolithic repository** containing multiple related projects managed together:

**Core Server:**
- `src/neonsignal/` - C++23 HTTP/2 server with TLS/SSL, virtual hosting, and WebAuthn
- `include/neonsignal/` - C++ header files
- `src/neonsignal/database/` - LIBMDBX embedded database integration

**Frontend Runtime & Applications:**
- `neonjsx/` - Custom JSX runtime implementation (not React)
- `www/neonsignaljsx/` - Main NeonSignal web application
- `www/nutsloopjsx/` - NutsLoop project website
- `www/simonedelpopolo/` - Personal blog with AI-powered content generation
- `www/_default/` - Fallback static site used by benchmarks and unknown hosts

**Documentation & Theming:**
- `www/book/` - Sphinx technical documentation
- `themes/sphinx-synthwave-theme/` - Custom synthwave documentation theme

**Infrastructure:**
- `scripts/` - Build automation, certificates, deployments, and benchmarks
- `systemd/` - Service unit files for production
- `public/` - Compiled output served by the HTTP/2 server

**Benefits of this structure:**
- Shared build system (Meson for C++, npm for TypeScript)
- Unified certificate management across all domains
- Single deployment pipeline for server and frontends
- Consistent development environment

## Prerequisites

### System Requirements

NeonSignal is developed and tested on Oracle Cloud Infrastructure:

- **OS:** Oracle Linux 10
- **Architecture:** ARM64 (aarch64)
- **Hardware:** Ampere A1 Compute
  - 3 OCPU (vCPU)
  - 16GB RAM
  - Oracle Cloud Always Free tier

### Check Compiler Version

Verify your system has a C++23 compatible compiler:

```bash
gcc --version
```

Expected output on Oracle Linux 10:

```
gcc (GCC) 14.2.1 20240801 (Red Hat 14.2.1-1)
Copyright (C) 2024 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### Enable Required Repositories

Enable Oracle Linux development repositories:

```bash
# Enable CodeReady Builder (development tools)
sudo dnf config-manager --set-enabled ol10_codeready_builder

# Enable UEK-next Developer Release (latest kernel features)
sudo dnf config-manager --set-enabled ol10_developer_UEKnext

# Enable EPEL for additional packages
sudo dnf config-manager --set-enabled ol10_u0_developer_EPEL
```

Node.js 22.x is required. If Oracle Linux repositories already provide Node 22 on your system, you can skip NodeSource. Otherwise, enable the NodeSource repository:

```bash
# Add NodeSource repository
curl -fsSL https://rpm.nodesource.com/setup_22.x | sudo bash -
```

Verify all repositories are enabled:

```bash
dnf repolist enabled
```

Expected output (if NodeSource is enabled):

```
repo id                repo name
nodesource-nodejs      Node.js Packages for Linux RPM based distros - aarch64
nodesource-nsolid      N|Solid Packages for Linux RPM based distros - aarch64
ol10_appstream         Oracle Linux 10 Application Stream Packages (aarch64)
ol10_baseos_latest     Oracle Linux 10 BaseOS Latest (aarch64)
ol10_codeready_builder Oracle Linux 10 CodeReady Builder (aarch64) - (Unsupported)
ol10_developer_UEKnext UEK-next Developer Release (aarch64)
ol10_u0_developer_EPEL Oracle Linux 10.0 EPEL Packages for Development (aarch64)
```

### Install Core Dependencies

Install the required packages:

```bash
sudo dnf install -y \
    gcc-c++ \
    meson \
    ninja-build \
    cmake \
    openssl-devel \
    libnghttp2-devel \
    git \
    nodejs
```

Verify Node.js version (minimum 22.21.1 required):

```bash
node --version
```

### Install OpenAI Codex CLI

NeonSignal uses OpenAI Codex for AI-powered features:

```bash
# Install globally via npm
npm install -g @openai/codex-cli

# Verify installation
codex --version
```

**API Access Setup:**

The Codex API requires authentication via ChatGPT login. If your API endpoint runs on a remote server, set up an SSH tunnel to access it locally:

```bash
# Create SSH tunnel to remote API server
# -N: No remote command execution
# -L: Local port forwarding (1455:127.0.0.1:1455)
# This forwards local port 1455 to the remote server's port 1455
ssh -N -L 1455:127.0.0.1:1455 theirs-ssh.server
```

**Why use SSH tunneling?**
- Securely access a remote Codex API endpoint through an encrypted SSH connection
- Bypass firewall restrictions by routing traffic through SSH
- Make a remote service appear as if it's running locally on port 1455
- Avoid exposing the API directly to the internet

Keep the SSH tunnel running in the background while using Codex. The CLI will connect to `localhost:1455`, which tunnels to the remote server.

## Building from Source

### Clone the Repository

```bash
git clone https://github.com/nutsloop/neonsignal.git
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

## Running NeonSignal

### Configure Firewall (Recommended)

Instead of running services with `sudo` on privileged ports (80, 443), use firewall rules to redirect traffic to non-privileged ports. This allows running NeonSignal as a regular user with better security.

```bash
# Redirect HTTPS traffic (443 → 9443)
sudo firewall-cmd --permanent --direct --add-rule ipv4 nat PREROUTING 0 -p tcp --dport 443 -j REDIRECT --to-port 9443

# Redirect HTTP traffic (80 → 9090)
sudo firewall-cmd --permanent --direct --add-rule ipv4 nat PREROUTING 0 -p tcp --dport 80 -j REDIRECT --to-port 9090

# Reload firewall to apply changes
sudo firewall-cmd --reload

# Verify rules
sudo firewall-cmd --direct --get-all-rules
```

**Benefits of this approach:**
- Run NeonSignal as non-root user (more secure)
- No `sudo` required for server processes
- Standard ports (80, 443) work transparently
- Firewall handles the redirection at kernel level

### Generate Development Certificates

For local development, generate self-signed certificates using the unified certificate issuer:

```bash
./scripts/certificates/issuer.sh --local generate-all
```

This creates:
- A local Certificate Authority (CA) in `certs/ca/`
- Certificates for configured `.host` domains in `certs/`

To trust the certificates in your browser, import `certs/ca/root.crt` as a trusted root certificate.

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

| Variable               | Default              | Description                           |
|------------------------|----------------------|---------------------------------------|
| `NEONSIGNAL_HOST`      | `0.0.0.0`            | Bind address                          |
| `NEONSIGNAL_PORT`      | `9443`               | HTTPS listen port                     |
| `NEONSIGNAL_THREADS`   | `3`                  | Worker thread count (match CPU cores) |
| `NEONSIGNAL_DB_PATH`   | `data/neonsignal.mdb`| LIBMDBX database file path            |
| `NEONSIGNAL_RP_ID`     | *(none)*             | WebAuthn Relying Party ID (required)  |
| `NEONSIGNAL_ORIGIN`    | *(none)*             | WebAuthn origin URL (required)        |

**Example (with firewall redirect configured):**

```bash
# Run on port 9443 (firewall redirects 443 → 9443)
# Uses 3 workers to match Oracle Cloud 3 OCPU
NEONSIGNAL_THREADS=3 ./build/src/neonsignal
```

External clients connect to standard port 443, firewall transparently redirects to 9443.

### Start the Redirect Service

The redirect service handles HTTP→HTTPS redirects and ACME challenges:

```bash
# Run on port 9090 (firewall redirects 80 → 9090)
./build/src/neonsignal_redirect
```

| Variable               | Default          | Description                     |
|------------------------|------------------|---------------------------------|
| `REDIRECT_HOST`        | `0.0.0.0`        | Bind address                    |
| `REDIRECT_PORT`        | `9090`           | HTTP listen port                |
| `REDIRECT_TARGET_PORT` | `443`            | HTTPS redirect target           |
| `REDIRECT_INSTANCES`   | `1`              | Worker count                    |
| `ACME_WEBROOT`         | `acme-challenge` | ACME challenge directory        |

External clients connect to standard port 80, firewall transparently redirects to 9090.

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

### Install Binaries to System Path

Copy the compiled binaries to `/usr/local/bin` for systemd services:

```bash
# Install binaries
sudo cp build/src/neonsignal /usr/local/bin/
sudo cp build/src/neonsignal_redirect /usr/local/bin/

# Set permissions
sudo chmod 755 /usr/local/bin/neonsignal
sudo chmod 755 /usr/local/bin/neonsignal_redirect
```

### Fix SELinux Context

On Oracle Linux with SELinux enabled, restore the correct security context for the binaries:

```bash
# Restore SELinux context for binaries
sudo restorecon -v /usr/local/bin/neonsignal
sudo restorecon -v /usr/local/bin/neonsignal_redirect

# Verify SELinux context
ls -Z /usr/local/bin/neonsignal*
```

Expected output:
```
system_u:object_r:bin_t:s0 /usr/local/bin/neonsignal
system_u:object_r:bin_t:s0 /usr/local/bin/neonsignal_redirect
```

### Configure Systemd Services

**IMPORTANT:** The systemd service files in `systemd/` directory **MUST be adjusted** to match your environment before installation.

Review and edit `systemd/neonsignal.service`:

```ini
[Service]
User=core                              # ← Change to your username
Group=core                             # ← Change to your group
WorkingDirectory=/home/core/code/neonsignal  # ← Change to your path
Environment=NEONSIGNAL_THREADS=3       # ← Match your CPU count
Environment=NEONSIGNAL_HOST=10.0.0.10  # ← Change to your IP
Environment=NEONSIGNAL_PORT=9443       # ← Adjust if needed
Environment=NEONSIGNAL_RP_ID=neonsignal.nutsloop.host     # ← Your domain
Environment=NEONSIGNAL_ORIGIN=https://neonsignal.nutsloop.host  # ← Your origin
ExecStart=/usr/local/bin/neonsignal
```

Review and edit `systemd/neonsignal-redirect.service`:

```ini
[Service]
User=core                              # ← Change to your username
Group=core                             # ← Change to your group
WorkingDirectory=/home/core/code/neonsignal  # ← Change to your path
Environment=REDIRECT_INSTANCES=3       # ← Match your CPU count
Environment=REDIRECT_PORT=9090         # ← Default HTTP redirect port
Environment=REDIRECT_TARGET_PORT=443   # ← HTTPS target port
Environment=REDIRECT_HOST=10.0.0.10    # ← Change to your IP
ExecStart=/usr/local/bin/neonsignal_redirect
```

Install the service files:

```bash
# Install service files with correct permissions
sudo install -m 0644 systemd/neonsignal.service /etc/systemd/system/neonsignal.service
sudo install -m 0644 systemd/neonsignal-redirect.service /etc/systemd/system/neonsignal-redirect.service
sudo systemctl daemon-reload
sudo systemctl enable neonsignal.service neonsignal-redirect.service
sudo systemctl start neonsignal.service neonsignal-redirect.service

# Check status
sudo systemctl status neonsignal.service
sudo systemctl status neonsignal-redirect.service
```

### Uninstall Services

To remove NeonSignal systemd services and binaries:

```bash
# Stop and disable services
sudo systemctl stop neonsignal.service neonsignal-redirect.service
sudo systemctl disable neonsignal.service neonsignal-redirect.service

# Remove service files
sudo rm -f /etc/systemd/system/neonsignal.service /etc/systemd/system/neonsignal-redirect.service
sudo systemctl daemon-reload

# Remove binaries from system path
sudo rm -f /usr/local/bin/neonsignal /usr/local/bin/neonsignal_redirect
```

For production deployment with Let's Encrypt certificates, see the [Deployment Guide](part4-operations/deployment).

## Directory Structure

```
neonsignal/
├── build/              # Build output (generated)
├── certs/              # TLS certificates
│   ├── _default/       # Fallback certificate
│   └── <domain>/       # Per-domain certificates
├── data/               # Runtime data
│   ├── codex/          # Codex AI workflow runs
│   │   └── runs/       # Individual run artifacts
│   ├── neonsignal.mdb  # LIBMDBX database file
│   └── neonsignal.mdb-lck  # Database lock file
├── public/             # Static files (document root)
│   ├── _default/       # Fallback content
│   └── <domain>/       # Per-domain content
├── src/                # C++ source code
├── include/            # Header files
├── scripts/            # Build and deployment scripts
├── systemd/            # Service unit files
└── www/book/           # Sphinx documentation source
```

## Next Steps

- Learn about [Virtual Hosting](part2-architecture/virtual-hosting) for multi-domain setups
- Set up [Let's Encrypt certificates](part4-operations/deployment) for production
- Explore [SSE streaming](part3-features/sse) for real-time features
