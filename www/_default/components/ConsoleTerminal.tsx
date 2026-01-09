import { h } from '@nutsloop/neonjsx';

/**
 * ConsoleTerminal - Terminal window component
 * The terminal body (id="terminal-output") will be targeted by console-stream.ts
 */
export const ConsoleTerminal = () => {
  return (
    <section class="terminal">
      <div class="terminal-header">
        <span class="terminal-dot" />
        <span class="terminal-dot" />
        <span class="terminal-dot" />
        <span class="terminal-title">WASTELAND RELAY v2.049</span>
      </div>
      <div class="terminal-body" id="terminal-output">
        {/* Content will be streamed via console-stream.ts */}
      </div>
    </section>
  );
};
