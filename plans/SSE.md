# Plan: Periodic SSE reset to free buffered data

## Goal
Allow SSE clients to drop accumulated data by periodically closing the SSE stream gracefully and letting EventSource auto-reconnect. This keeps the browser buffer small without changing client code.

## Approach
1. Track per-SSE-channel message counts/timestamps on the server.
2. After a threshold (e.g., N messages or T seconds), send a final SSE comment (optional) and close the stream (END_STREAM). The client EventSource will reconnect automatically.
3. Keep behavior transparent: no protocol change; no new client code required.

## Files to modify
- `src/neonsignal/http2_listener/handle_io_.c++`
  - Add per-connection counters/timestamps for SSE streams (events/cpu/mem/redirect).
  - When sending SSE DATA frames, increment counter and check threshold; if exceeded, enqueue an empty DATA frame with END_STREAM (or a comment) to close the stream.
  - Ensure connection flags are reset on close, allowing a new stream on reconnect.
- `include/neonsignal/http2_listener.h++` (if needed)
  - Extend `Http2Connection` stream state to track SSE send count/last-reset time per SSE channel.
- `src/neonsignal/http2_listener/handle_io_.c++` (DATA path)
  - When streaming to existing SSE clients (loop over `conns_`), check/reset logic before appending DATA frames.

## Thresholds (tunable)
- Add a configuration struct for SSE reset policy:
  - Time-based (default): reset after T seconds (default 45s).
  - Message-based: reset after N messages (e.g., 200).
  - Mode selector: `only_time`, `only_n_messages`, or `both` (both = reset if either threshold is exceeded).
- Make the mode selectable (e.g., enum) and default to `only_time` with 45s.

## Safety/compatibility
- HTTP/2: use END_STREAM on the SSE stream; connection stays open.
- Browser: EventSource auto-reconnects; no client changes needed.
- Non-SSE APIs and HTML unaffected.

## Testing
- Connect via EventSource to each channel, observe periodic close/reconnect.
- Verify buffers shrink (DevTools) and data continues after reconnect.
- Ensure no infinite loop of immediate reconnects (use time threshold).
