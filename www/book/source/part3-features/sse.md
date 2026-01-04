# Real-time Events (SSE)

Neonsignal uses Server-Sent Events (SSE) to stream real-time data to clients for features like live monitoring dashboards.

## SSE Stream Reset

A key challenge with long-lived SSE streams is that client-side buffers can grow indefinitely, consuming memory in the browser. To mitigate this, Neonsignal implements a periodic reset mechanism.

### Approach

1.  **Server-Side Tracking:** The server tracks the number of messages sent or the elapsed time for each active SSE stream.
2.  **Threshold Trigger:** When a pre-defined threshold is met (e.g., 45 seconds have passed), the server gracefully closes the HTTP/2 stream by sending a final empty DATA frame with the `END_STREAM` flag set.
3.  **Automatic Reconnection:** The client-side `EventSource` API is designed to automatically reconnect when a stream is closed. This behavior is leveraged to seamlessly establish a new stream.

This process is transparent to the client application and requires no custom client-side code. It effectively allows the browser to garbage-collect the old buffer and start fresh, keeping memory usage low without interrupting the flow of data.

### Configurable Thresholds

The reset policy is configurable on the server:

-   **Time-based (default):** Reset after a specific duration (e.g., 45 seconds).
-   **Message-based:** Reset after a specific number of messages (e.g., 200).
-   **Hybrid:** Reset when either threshold is met.
