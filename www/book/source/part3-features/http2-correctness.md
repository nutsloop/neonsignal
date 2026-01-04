# HTTP/2 Correctness

Given Neonsignal's custom HTTP/2 stack, ensuring protocol correctness is critical for security and stability. The following plan outlines the measures taken to harden the implementation.

## Hardening Areas

The focus of correctness improvements is on these key areas:

-   **Frame I/O & Parsing:** Handling of `HEADERS`, `DATA`, `WINDOW_UPDATE`, and `RST_STREAM`/`GOAWAY` frames.
-   **HPACK Decoding:** Securely parsing compressed headers.
-   **Flow Control:** Managing per-connection and per-stream window updates.
-   **Header Validation:** Enforcing rules for pseudo-headers and general header syntax.
-   **Uploads:** Managing `DATA` frame aggregation and size limits.

## Conformance Testing

To validate the implementation against the HTTP/2 specification, the following testing strategies are employed:

1.  **h2spec/h2load:** The `h2spec` tool is used to run a suite of conformance tests against the server. A `scripts/http2_conformance.sh` script automates this process. `h2load` is used for basic smoke testing.
2.  **Nghttp2 Client Integration Test:** A custom test harness using the `nghttp2` library is used to test specific scenarios, such as valid requests (200 OK, 404 Not Found), malformed requests (bad pseudo-header order, oversized headers), and error conditions.

## Guardrails and Best Practices

The following guardrails are implemented in the `handle_io_.c++` request handler to prevent common attacks and protocol violations:

-   **Pseudo-header Enforcement:** The server rejects requests with:
    -   Missing `:path` or `:method` headers.
    -   Duplicated pseudo-headers.
    -   Pseudo-headers appearing after regular headers.
    -   Uppercase header names.
-   **Size Limits:** Header block size and total `DATA` per stream (for uploads) are capped to prevent resource exhaustion. Violations result in an HTTP 431 or 413 error, respectively.
-   **Flow Control:** The server strictly tracks connection and stream-level flow control windows, sending `WINDOW_UPDATE` frames as it consumes data. It closes connections that violate flow control rules (e.g., sending data beyond the window size).
-   **Frame Size:** The server enforces the maximum frame size advertised by the client.
-   **Error Handling:** On a protocol violation, the server emits a `GOAWAY` frame with the appropriate error code and closes the connection cleanly.

## Logging and Metrics

To provide visibility into the health of the HTTP/2 stack, the server includes counters and logging for:

-   Malformed headers
-   HPACK decoding failures
-   Flow control violations
-   Frame size violations
-   `GOAWAY` events, including the reason for termination.
