import { Footer } from './components/footer';

export const AboutPage = () => (
  <div className="page landing">
    <section className="hero landing-hero">
      <div className="glow" />
      <div className="content">
        <p className="signal-id">Transmission: ABOUT</p>
        <h1 data-text="NEONSIGNAL">NEONSIGNAL</h1>
        <p className="subtitle">About this relay</p>
        <p className="lede">
          NeonSignal is a fast HTTP/2-only server built for deterministic latency, modern TLS,
          and an observability-first control plane.
        </p>
        <div className="feature-grid">
          <div className="feature">
            <span className="feature-icon">HPACK</span>
            <span className="feature-text">HPACK + nghttp2 decoding</span>
          </div>
          <div className="feature">
            <span className="feature-icon">THREAD</span>
            <span className="feature-text">Multi-threaded epoll core</span>
          </div>
          <div className="feature">
            <span className="feature-icon">SSE</span>
            <span className="feature-text">Live SSE telemetry</span>
          </div>
          <div className="feature">
            <span className="feature-icon">AUTH</span>
            <span className="feature-text">WebAuthn admin access</span>
          </div>
        </div>
        <div className="cta-row">
          <a href="/" className="pill">Back to Home</a>
          <a href="/auth.html" className="pill primary" target="_blank">Admin Panel</a>
        </div>
      </div>
    </section>
    <Footer />
  </div>
);
