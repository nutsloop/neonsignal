========
The Book
========

.. include:: _notice/_toc.birthday.rst


.. raw:: html

   <hr>

.. raw:: html

   <h6>HTTP/2 asynchronous server for real-time applications.</h6>

----

**neonsignal** is a high-performance server written in modern `C++23`.

Built as a monolithic repository that integrates:
  - Backend server
  - Sphinx documentation
  - AI-powered analysis

.. note::
   **Target Platform:** Oracle Linux 10 (ARM64) on Oracle Cloud Infrastructure (Ampere A1 Compute)

.. grid:: 2
   :gutter: 3

   .. grid-item-card:: Quick Start
      :link: getting-started
      :link-type: doc

      Build from source and run a server.

   .. grid-item-card:: Architecture
      :link: part2-architecture/index
      :link-type: doc

      Understand the core design: event loops, HTTP/2 frames, and virtual hosting.

   .. grid-item-card:: Features
      :link: part3-features/index
      :link-type: doc

      Explore SSE streaming, performance tuning, and HTTP/2 compliance.

   .. grid-item-card:: Deployment
      :link: part4-operations/deployment
      :link-type: doc

      Production setup with systemd, Let's Encrypt, and monitoring.

Key Features
------------

Core Server
^^^^^^^^^^^

- **HTTP/2 native** — custom implementation with nghttp2 HPACK decode
- **TLS 1.3+** — Modern cryptography via OpenSSL
- **SNI Virtual Hosting** — Per-domain certificates and content
- **Server-Sent Events** — Real-time streaming with automatic buffer management
- **LIBMDBX Database** — Embedded transactional storage
- **WebAuthn Support** — User enrollment and authentication
- **Multiple Frontends** — Web applications for different domains
- **~1MB Binaries** — Compact, stripped, release builds (902K + 196K)

Project Stack
^^^^^^^^^^^^^^^^

- **NeonJSX Runtime** — Custom JSX implementation (`neonjsx <https://github.com/nutsloop/neonjsx.js>`_)
- **Sphinx Documentation** — Technical docs with custom synth-wave theme

Ecosystem Implementation Status
-------------------------------

.. list-table::
   :header-rows: 1
   :widths: 50 20

   * - Feature
     - Status
   * - server APIs are hardcoded
     - ✦ Future Vision
   * - HTTP/2 Server (C++23)
     - ✔ Working
   * - TLS 1.3 / SNI Certificates
     - ✔ Working
   * - Virtual Hosting (directory-based)
     - ✔ Working
   * - Static File Serving
     - ✔ Working
   * - SSE Streaming
     - ✔ Working
   * - LIBMDBX Database Integration
     - ✔ Working
   * - WebAuthn User Enrollment
     - ✔ Working
   * - Let's Encrypt Integration
     - ✔ Working
   * - NeonJSX Runtime
     - ✔ Working
   * - Sphinx Documentation
     - ✔ Working
   * - VHScript Configuration
     - … Planned
   * - NeonEcho Language
     - ✦ Future Vision

----

.. toctree::
   :maxdepth: 2
   :caption: Getting Started

   getting-started

.. toctree::
   :maxdepth: 2
   :caption: Part 1

   part1-philosophy/index
   part1-philosophy/neonecho

.. toctree::
   :maxdepth: 2
   :caption: Part 2

   part2-architecture/index
   part2-architecture/virtual-hosting

.. toctree::
   :maxdepth: 2
   :caption: Part 3

   part3-features/index
   part3-features/sse
   part3-features/performance
   part3-features/http2-correctness

.. toctree::
   :maxdepth: 2
   :caption: Part 4

   part4-operations/index
   part4-operations/deployment
   part4-operations/benchmarking

.. toctree::
   :maxdepth: 2
   :caption: Part 5

   part5-project-health/index

.. toctree::
   :maxdepth: 1
   :caption: Benchmarks

   benchmarks/index

.. toctree::
   :maxdepth: 1
   :caption: AI Conversations

   ai-conversations/index
