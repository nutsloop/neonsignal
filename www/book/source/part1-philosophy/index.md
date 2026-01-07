# The NeonSignal Philosophy

```{epigraph}
"We don't abstract away the machine—we embrace it."

— nutsloop collective
```

Welcome to the official documentation for NeonSignal, a high-performance HTTP/2 server engineered for scenarios where every millisecond matters. This book serves as a comprehensive guide to its architecture, features, and operational philosophy.

## Origin Story

NeonSignal emerged from a deliberate rejection of the "good enough" mentality that pervades modern web development. In a landscape dominated by interpreted runtimes and framework abstractions, we asked a different question: *What if we built a web server the way game engines are built?*

The result is a server written in C++23—not because it's easy, but because it's fast. Every component, from the epoll-based event loop to the custom JSX runtime, exists because we chose control over convenience.

```{note}
This project was developed with ~99% AI assistance through iterative collaboration, demonstrating that cutting-edge tooling can accelerate even the most complex systems programming.
```

---

## The NeonSignal Philosophy

The architecture of NeonSignal is engineered for scenarios where performance, low latency, and resource efficiency are paramount. The project's philosophy consistently favors **control and speed** over developer convenience and rapid iteration.

### Core Tenets

```{list-table}
:header-rows: 1
:widths: 20 80

* - Tenet
  - Description
* - **Performance-First**
  - Built directly on a low-level `epoll`-based event loop. No framework overhead. No runtime interpretation. Raw system calls wrapped in modern C++23.
* - **Single-Threaded Core**
  - All socket I/O stays on the main thread. No mutexes. No race conditions. ThreadPool reserved only for truly blocking operations.
* - **Modern Feature Set**
  - HTTP/2 multiplexing, TLS 1.3+, WebAuthn passwordless authentication, Server-Sent Events with automatic stream reset, embedded LIBMDBX storage.
* - **Minimal Dependencies**
  - Three external libraries: `OpenSSL`, `libnghttp2`, `libmdbx`. No dependency hell. No supply chain anxiety.
* - **Custom Everything**
  - JSX runtime (2KB vs React's 40KB+), SSE broadcaster with pre-encoded frames, connection manager with DoS protection—all purpose-built.
```

### The Single-Threaded Decision

NeonSignal's event loop runs entirely on the main thread. This isn't a limitation—it's a feature.

```
┌─────────────────────────────────────────────────────────────────┐
│                     Main Thread (Listener)                      │
│                                                                 │
│   ┌─────────────────────────────────────────────────────────┐   │
│   │                  EventLoop (epoll)                      │   │
│   │                                                         │   │
│   │   ┌──────────┐  ┌──────────┐  ┌──────────┐             │   │
│   │   │  Accept  │  │  I/O per │  │  Timers  │             │   │
│   │   │ Callback │  │Connection│  │ Callback │             │   │
│   │   └──────────┘  └──────────┘  └──────────┘             │   │
│   │                                                         │   │
│   └─────────────────────────────────────────────────────────┘   │
│                              │                                  │
│                              ▼                                  │
│   ┌─────────────────────────────────────────────────────────┐   │
│   │              ThreadPool (Optional)                      │   │
│   │         For blocking tasks only (heavy compute)         │   │
│   └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

**Why this matters:**
- No mutex contention on hot paths
- No race conditions in connection state
- Predictable latency (no lock waiting)
- Simplified debugging (single call stack)

This is the same model that powers Node.js and nginx—proven at scale.

---

## Measured Performance

These aren't aspirational numbers. They're measured on production hardware.

```{list-table}
:header-rows: 1
:widths: 40 60

* - Metric
  - Value
* - Binary Size
  - ~1MB total (902K server + 196K redirector, stripped)
* - Throughput
  - ~8,700 requests/second (h2load benchmark)
* - Mean Latency
  - 11.35ms on ARM64
* - Max Connections
  - 10,000 concurrent
* - SSE Batching
  - ~60fps (16ms intervals)
* - Static Cache
  - 50MB LRU with ETag validation
* - Codebase
  - ~96 C++ files, 540K source, 140K headers
```

**Target Platform:** Oracle Linux 10 (ARM64) on Oracle Cloud Infrastructure Ampere A1 Compute.

---

## Trade-offs: The Honest Truth

Every architectural decision is a trade-off. Here's what you gain—and what you give up.

### Comparison with General-Purpose Frameworks

```{list-table}
:header-rows: 1
:widths: 25 25 25 25

* - Aspect
  - NeonSignal
  - Express/Fastify
  - Django/Rails
* - Request Latency
  - <15ms
  - 50-150ms
  - 100-300ms
* - Memory per Connection
  - ~2KB
  - 50-200KB
  - 100-500KB
* - Binary/Runtime Size
  - ~1MB
  - 50-100MB (Node)
  - 200MB+ (Python/Ruby)
* - Startup Time
  - <100ms
  - 2-10s
  - 5-30s
* - Learning Curve
  - Steep
  - Moderate
  - Gentle
* - Ecosystem
  - Minimal
  - Extensive
  - Extensive
* - Team Scalability
  - Limited
  - High
  - High
```

### The Trade-off Visualization

```
      ▲ Performance
      │
      │   ┌─────────────────────────────────────┐
      │   │                                     │
  High│   │           NeonSignal                │
      │   │     (control, speed, efficiency)    │
      │   │                                     │
      │   └─────────────────────────────────────┘
      │
      │
      │                     ┌─────────────────────────────────────┐
      │                     │                                     │
  Mid │                     │     Actix, Axum, Fastify            │
      │                     │    (performance + some ecosystem)   │
      │                     │                                     │
      │                     └─────────────────────────────────────┘
      │
      │                                         ┌─────────────────────────────────────┐
      │                                         │                                     │
  Low │                                         │     Express, Django, Rails          │
      │                                         │    (ecosystem + developer speed)    │
      │                                         │                                     │
      │                                         └─────────────────────────────────────┘
      │
      └────────────────────────────────────────────────────────────────────────────────►
                           Developer Experience & Ecosystem                        High
```

---

## The Cost of Control

Choosing NeonSignal means accepting certain constraints. This isn't a criticism—it's informed consent.

### What You Give Up

```{list-table}
:header-rows: 1
:widths: 30 70

* - Convenience
  - Reality
* - Middleware Ecosystem
  - No `npm install express-session`. Every feature is built from scratch or doesn't exist.
* - ORM
  - Raw LIBMDBX queries. No ActiveRecord, no Prisma, no magic.
* - Template Engine
  - Custom JSX runtime or static HTML. No Jinja, no EJS.
* - Hot Reload
  - Recompile the binary. Every time.
* - Stack Overflow Answers
  - This book is your documentation. The codebase is your reference.
* - Junior-Friendly Onboarding
  - New developers need C++, async I/O, HTTP/2, and TLS knowledge.
```

### What You Gain

- **Predictable Performance:** No GC pauses. No JIT warmup. No cold starts.
- **Complete Control:** Every byte that leaves the server, you wrote the code for.
- **Minimal Attack Surface:** Three dependencies. Auditable in an afternoon.
- **Single Binary Deployment:** Copy one file. Run it. Done.

---

## When to Use NeonSignal

NeonSignal excels in specific scenarios. Know your use case.

### Ideal Use Cases

```{admonition} Edge Server / API Gateway
:class: tip

- SNI-based virtual hosting (multiple domains, single binary)
- Pre-encoded SSE frame broadcasting to thousands of clients
- Static file caching with LRU eviction and ETag validation
- Connection limiting and DoS protection built-in
```

```{admonition} Real-Time Applications
:class: tip

- SSE streams with automatic reset for browser memory efficiency
- ~60fps event batching (16ms intervals)
- WebAuthn passwordless authentication
- Sub-15ms response latency
```

```{admonition} Resource-Constrained Environments
:class: tip

- ARM64 optimization (tested on Oracle Cloud Ampere A1)
- ~1MB total binary size
- ~2KB memory per connection
- No JVM, no V8, no interpreter overhead
```

### When NOT to Use NeonSignal

```{warning}
NeonSignal is not the right choice for every project. Be honest about your constraints.
```

- **Rapid Prototyping:** If time-to-market matters more than latency, use Express or FastAPI.
- **Teams Without C++ Expertise:** This is not a learning project. Production debugging requires systems knowledge.
- **Extensive Third-Party Integrations:** No Stripe SDK. No AWS SDK. Roll your own or find another solution.
- **Frequent Developer Onboarding:** Every new team member needs weeks to become productive.
- **Compliance-Heavy Environments:** No SOC2 certification. No enterprise support contract.

---

## The nutsloop Philosophy

NeonSignal is more than a technical project—it's a statement about how software should be built.

### Building the Future

```
"In the neon glow of the digital frontier,
 where packets flow like streams of light,
 we build the infrastructure of tomorrow."
```

The synthwave aesthetic isn't just visual decoration. It represents a philosophy:

- **Retro-Futurism:** Looking back at the optimism of early computing while building for tomorrow.
- **Craftsmanship:** Every component hand-built, not assembled from off-the-shelf parts.
- **Long-Term Thinking:** Architecture decisions made for decades, not sprints.
- **Performance as Art:** Speed isn't just functional—it's beautiful.

### The Collective

The nutsloop organization exists to build infrastructure that matters. NeonSignal is the first public artifact of this mission—a server that proves you can have both speed and style, both control and elegance.

We don't compete with frameworks. We build for a different audience: developers who see their servers as craftsmanship, not commodities.

---

## What's Next

Continue to [NeonEcho](neonecho.md) to explore the aspirational custom scripting language designed for NeonSignal's future.
