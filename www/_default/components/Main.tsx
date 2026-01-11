import { ConsoleTerminal } from './ConsoleTerminal';

export const Main = () => {
  return (
    <>
      {/* Synthwave animated background */}
      <div class="synthwave-bg" />

      {/* CRT scanline overlay */}
      <div class="crt-overlay" />

      {/* Main content */}
      <div class="main-container">
        {/* Neon title with flicker effect */}
        <header>
          <h1 class="neon-title glitch" data-text="NEON SIGNAL">
            <span class="accent">NEON</span> SIGNAL
          </h1>
          <p class="subtitle">transmissions from the wasteland</p>
        </header>

        {/* Console terminal */}
        <ConsoleTerminal />

        {/* Status bar */}
        <div class="status-bar">
          <div class="status-item">
            <span class="status-dot" />
            <span>UPLINK ACTIVE</span>
          </div>
          <div class="status-item">
            <span class="status-dot warning" />
            <span>SIGNAL: 87%</span>
          </div>
          <div class="status-item">
            <span class="status-dot" />
            <span>SECTOR: ALPHA-7</span>
          </div>
        </div>
      </div>
    </>
  );
};
