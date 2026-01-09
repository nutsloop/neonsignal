import { Footer } from './components/footer';

const _ASCII_ART = `
    ███╗   ██╗███████╗ ██████╗ ███╗   ██╗
    ████╗  ██║██╔════╝██╔═══██╗████╗  ██║
    ██╔██╗ ██║█████╗  ██║   ██║██╔██╗ ██║
    ██║╚██╗██║██╔══╝  ██║   ██║██║╚██╗██║
    ██║ ╚████║███████╗╚██████╔╝██║ ╚████║
    ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝

███████╗██╗ ██████╗ ███╗   ██╗ █████╗ ██╗
██╔════╝██║██╔════╝ ████╗  ██║██╔══██╗██║
███████╗██║██║  ███╗██╔██╗ ██║███████║██║
╚════██║██║██║   ██║██║╚██╗██║██╔══██║██║
███████║██║╚██████╔╝██║ ╚████║██║  ██║███████╗
╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝

`;

export const LandingPage = () => (
  <div className="page landing">
    <section className="hero landing-hero">
      <div className="glow" />
      <div className="landing-sheen" aria-hidden="true" />
      <div className="landing-sparkles" aria-hidden="true">
        <span className="sparkle s1" />
        <span className="sparkle s2" />
        <span className="sparkle s3" />
        <span className="sparkle s4" />
        <span className="sparkle s5" />
        <span className="sparkle s6" />
      </div>
      <div className="content">
        <p className="signal-id">Signal ID: NS-CORE-01</p>
        <h1 data-text="NEONSIGNAL">NEONSIGNAL</h1>
        {/*<pre className="ascii-art" aria-label="NeonSignal ASCII Art">{ASCII_ART}</pre>*/}
        <p className="tagline">High-performance HTTP/2 server</p>
        <p className="subtitle">Deterministic latency for the wasteland mesh</p>
        <div className="birthday-banner">
          <span className="birthday-label">Project Birthday</span>
          <span className="birthday-date">November 25, 2025</span>
        </div>
        <div className="feature-grid">
          <div className="feature">
            <span className="feature-icon">IO</span>
            <span className="feature-text">Async epoll I/O</span>
          </div>
          <div className="feature">
            <span className="feature-icon">TLS</span>
            <span className="feature-text">TLS 1.3 + ALPN</span>
          </div>
          <div className="feature">
            <span className="feature-icon">H2</span>
            <span className="feature-text">HTTP/2 native</span>
          </div>
          <div className="feature">
            <span className="feature-icon">CACHE</span>
            <span className="feature-text">Static file cache</span>
          </div>
        </div>
        <div className="cta-row">
          <a href="/auth.html" className="pill primary" target="_blank">Admin Panel</a>
          <a href="/neonsignal-book.html" className="pill" target="_blank">Book</a>
          <a href="/about.html" className="pill">About</a>
          <a href="https://github.com/nutsloop/neonsignal" className="pill" target="_blank" rel="noopener">GitHub</a>
        </div>
        <div className="stats-mini">
          <span className="stat">
            <span className="stat-value" id="landing-reqs">~9.5k</span>
            <span className="stat-label">req/s</span>
          </span>
          <span className="stat">
            <span className="stat-value">~11ms</span>
            <span className="stat-label">latency</span>
          </span>
          <span className="stat">
            <span className="stat-value">TLS 1.3</span>
            <span className="stat-label">encryption</span>
          </span>
        </div>
      </div>
    </section>
    <Footer />
  </div>
);
