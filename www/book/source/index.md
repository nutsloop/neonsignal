# NeonSignal Documentation

**NeonSignal** is a high-performance HTTP/2 server written in modern C++23, designed for low-latency, real-time applications. Built as a monolithic repository, it integrates a C++23 server backend with custom JSX frontend runtime, Sphinx documentation, and AI-powered content generation.

**Target Platform:** Oracle Linux 10 (ARM64) on Oracle Cloud Infrastructure (Ampere A1 Compute)

::::{grid} 2
:gutter: 3

:::{grid-item-card} Quick Start
:link: getting-started
:link-type: doc

Build from source and run your first NeonSignal server in minutes.
:::

:::{grid-item-card} Architecture
:link: part2-architecture/index
:link-type: doc

Understand the core design: event loops, HTTP/2 frames, and virtual hosting.
:::

:::{grid-item-card} Features
:link: part3-features/index
:link-type: doc

Explore SSE streaming, performance tuning, and HTTP/2 compliance.
:::

:::{grid-item-card} Deployment
:link: part4-operations/deployment
:link-type: doc

Production setup with systemd, Let's Encrypt, and monitoring.
:::

::::

## Key Features

**Core Server:**
- **HTTP/2 Native** — Built from scratch on RFC 9113, not a wrapper
- **TLS 1.3+** — Modern cryptography via OpenSSL
- **SNI Virtual Hosting** — Per-domain certificates and content
- **Server-Sent Events** — Real-time streaming with automatic buffer management
- **LIBMDBX Database** — Embedded transactional storage
- **WebAuthn Support** — User enrollment and authentication
- **~1MB Binaries** — Compact, stripped, release builds (902K + 196K)

**Integrated Stack:**
- **NeonJSX Runtime** — Custom JSX implementation (not React)
- **Multiple Frontends** — Web applications for different domains
- **AI-Powered Content** — Codex CLI integration for blog generation
- **Sphinx Documentation** — Technical docs with custom synthwave theme
- **Unified Build System** — Meson (C++) + npm (TypeScript/JSX)

## Implementation Status

| Feature                           | Status            |
|-----------------------------------|-------------------|
| HTTP/2 Server (C++23)             | ✔ Working         |
| TLS 1.3 / SNI Certificates        | ✔ Working         |
| Virtual Hosting (directory-based) | ✔ Working         |
| Static File Serving               | ✔ Working         |
| SSE Streaming                     | ✔ Working         |
| LIBMDBX Database Integration      | ✔ Working         |
| WebAuthn User Enrollment          | ✔ Working         |
| Let's Encrypt Integration         | ✔ Working         |
| NeonJSX Runtime                   | ✔ Working         |
| AI Content Generation (Codex)     | ✔ Working         |
| Sphinx Documentation              | ✔ Working         |
| VHScript Configuration            | … Planned         |
| NeonEcho Language                 | ✦ Future Vision   |

---

```{toctree}
:maxdepth: 2
:caption: Getting Started

getting-started
```

```{toctree}
:maxdepth: 2
:caption: Part 1: The NeonSignal Philosophy

part1-philosophy/index
part1-philosophy/neonecho
```

```{toctree}
:maxdepth: 2
:caption: Part 2: Core Architecture

part2-architecture/index
part2-architecture/virtual-hosting
```

```{toctree}
:maxdepth: 2
:caption: Part 3: Features & Implementation

part3-features/index
part3-features/sse
part3-features/performance
part3-features/http2-correctness
```

```{toctree}
:maxdepth: 2
:caption: Part 4: Operations & Deployment

part4-operations/index
part4-operations/deployment
part4-operations/benchmarking
```

```{toctree}
:maxdepth: 2
:caption: Part 5: Project Health & Appendix

part5-project-health/index
```

```{toctree}
:maxdepth: 1
:caption: Benchmarks

benchmarks/index
```

<!-- AI CONVERSATIONS START -->
```{toctree}
:maxdepth: 1
:caption: AI Conversations

ai-conversations/index
```
<!-- AI CONVERSATIONS END -->
