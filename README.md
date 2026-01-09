# NeonSignal

A high-performance HTTP/2 server written in modern C++23, designed for low-latency, real-time applications. Built as a monolithic repository integrating a C++23 server backend with custom JSX frontend runtime, Sphinx documentation, and AI-powered content generation.

**Target Platform:** Oracle Linux 10 (ARM64) on Oracle Cloud Infrastructure (Ampere A1 Compute)
**Project Birthday:** November 25, 2025

## What is NeonSignal?

NeonSignal is a from-scratch HTTP/2 server implementation built on RFC 9113, leveraging modern C++23 features including coroutines, concepts, and ranges. The project combines a native HTTP/2 server with TLS 1.3+ support, SNI-based virtual hosting, embedded LIBMDBX database storage, WebAuthn passwordless authentication, and server-sent events for real-time streaming.

The monolithic repository integrates multiple components:

- **C++23 HTTP/2 Server** — Built with epoll-based event handling and nghttp2 for frame processing, delivering compact binaries (~1MB total) with production-ready connection management and DoS protection.

- **NeonJSX Runtime** — A custom JSX implementation with lightweight virtual DOM, not based on React, powering multiple frontend applications across different virtual hosts.

- **Embedded Database** — LIBMDBX transactional storage for users, sessions, and application data, providing ACID guarantees without external database dependencies.

- **WebAuthn Support** — Database-backed user registration and verification, followed by passkey/security key enrollment for passwordless authentication.

- **AI-Powered Content Generation** — Integration with OpenAI Codex CLI for automated blog post generation and content workflows.

- **Multi-Domain Hosting** — Directory-based virtual hosting with per-domain TLS certificates, static file caching with ETag support, and automatic Let's Encrypt certificate management.

- **Real-Time Features** — Server-Sent Events streaming with batched pre-encoded HTTP/2 frames, supporting high-frequency updates at approximately 60fps.

The project demonstrates practical application of C++23 features in systems programming, achieving high throughput (~8,700 req/s) with low latency (mean 11.35ms) on modest ARM64 hardware.

## Documentation

📖 **[Full Documentation](https://neonsignal.nutsloop.com/book/)**

Complete guides covering:
- Getting Started — Build instructions, dependencies, and deployment for Oracle Linux 10
- Architecture — Event loop design, HTTP/2 implementation, and virtual hosting
- Features — SSE streaming, performance tuning, and HTTP/2 compliance
- Operations — Production deployment with systemd, Let's Encrypt, and monitoring
- Benchmarks — Performance analysis and optimization techniques

## Contributing

We welcome contributions to NeonSignal! Please follow these guidelines:

### Pull Requests

1. **Fork and Branch** — Create a feature branch from `main` for your changes
2. **Follow Code Style** — C++ uses 2-space indentation, private members suffixed with `_`, snake_case file naming
3. **Test Your Changes** — Manually test HTTP/2 functionality and ensure no regressions
4. **Write Clear Commits** — Use conventional prefixes (`feat:`, `fix:`, `chore:`) with descriptive messages
5. **Update Documentation** — If adding features, update relevant Sphinx documentation files

See [CLAUDE.md](./CLAUDE.md) for detailed build instructions and project structure.

### Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/version/2/1/code_of_conduct/). We are committed to providing a welcoming and inclusive environment for all contributors. Please be respectful, considerate, and professional in all interactions.

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](./LICENSE) for details.

---

**Note:** This project was developed with approximately 99% AI assistance through iterative collaboration with AI coding agents.
