# Part 1: The NeonSignal Philosophy

Welcome to the official documentation for Neonsignal, a high-performance HTTP/2 server designed for specific, performance-critical use cases. This book serves as a comprehensive guide to its architecture, features, and operational procedures.

## The Neonsignal Philosophy

The architecture of Neonsignal strongly suggests that it is engineered for scenarios where performance, low latency, and resource efficiency are paramount. The project's philosophy consistently favors control and speed over developer convenience and rapid iteration.

### Core Tenets

- **Performance-First:** By building directly on a low-level `epoll`-based event loop, the server achieves exceptionally high performance.
- **Modern Feature Set:** The server includes support for HTTP/2, TLS 1.3+, WebAuthn for passwordless authentication, and Server-Sent Events (SSE) for real-time data streaming.
- **Minimal Dependencies:** The backend relies on a small, focused set of external libraries (`OpenSSL`, `libnghttp2`), reducing the surface area for vulnerabilities and simplifying the build process.
- **Custom-Tailored Solution:** Every component, from the C++ backend to the custom JSX runtime, is purpose-built to maximize performance and control.

### Strengths and Weaknesses

**Pros:**

- **Exceptional Performance:** Well-suited for high-traffic applications.
- **Modern Features:** A forward-looking and secure web server.
- **Minimal Dependencies:** Reduces vulnerabilities and simplifies builds.
- **Custom-Tailored:** Designed for performance and control.

**Cons:**

- **High Complexity:** The low-level C++ implementation is inherently complex to maintain.
- **Niche Expertise Required:** Requires deep knowledge of C++, async I/O, and HTTP/2.
- **Limited Ecosystem:** Lacks a rich ecosystem of third-party libraries and tools.
- **Potential for Reinventing the Wheel:** The custom JSX runtime is an example of building a component that is readily available elsewhere.

### Trade-offs

The following graph illustrates the trade-offs made in the Neonsignal project:

```
      ▲ High
      │
P     │   ┌─────────────────┐
e     │   │   Neonsignal    │
r     │   └─────────────────┘
f     │
o     │
r     │
m     │
a     │
n     │   ┌─────────────────┐
c     │   │  General-Purpose│
e     │   │   Frameworks    │
      │   └─────────────────┘
      │
      └──────────────────────────────────► High
        Developer Experience & Ecosystem
```

This server is an excellent choice for:

- A high-performance edge server or API gateway.
- Powering real-time applications that require low-latency data streaming.
- Environments where minimizing resource consumption is a primary concern.

The project's success is heavily tied to the availability of core developers with the necessary expertise. It is not a project that can be easily handed off to a generalist development team.
