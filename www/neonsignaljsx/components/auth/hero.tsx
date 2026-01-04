import { Login } from './login';

type HeroProps = {
  authenticated: boolean;
  onLoginSuccess?: () => void;
};

export const Hero = ( { authenticated, onLoginSuccess }: HeroProps ) => {

  const statusText = authenticated ? 'Authenticated' : 'Auth required';
  const statusColor = authenticated ? '#7CFF4D' : '#FF4D8D';
  const statusIcon = `data:image/svg+xml,${encodeURIComponent(
    `<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 18 18">
      <defs>
        <radialGradient id="g" cx="50%" cy="50%" r="60%">
          <stop offset="0%" stop-color="${statusColor}" stop-opacity="1"/>
          <stop offset="70%" stop-color="${statusColor}" stop-opacity="0.4"/>
          <stop offset="100%" stop-color="${statusColor}" stop-opacity="0"/>
        </radialGradient>
      </defs>
      <circle cx="9" cy="9" r="8" fill="${statusColor}" />
      <circle cx="9" cy="9" r="8" fill="url(#g)" />
      <circle cx="9" cy="9" r="4" fill="#0c0c14" opacity="0.6"/>
    </svg>`
  )}`;

  return (
    <section className="hero">
      <div className="glow" />
      <div className="content">
        <p className="signal-id">Signal: ADMIN-CORE</p>
        <h1 data-text="NEONSIGNAL">NEONSIGNAL</h1>
        <p className="lede">
          An async, epoll-driven HTTP/2 server with TLS ALPN, static routing, and
          nghttp2-backed HPACK decoding. Built for resilience in the post-apocalyptic mesh.
        </p>
        <div className="cta-row">
          <span className="pill primary">h2 | TLS 1.2+</span>
          <span className="pill">Epoll</span>
          <span className="pill">OpenSSL</span>
          <span className="pill status-pill" aria-label={statusText}>
            <img
              src={statusIcon}
              alt={statusText}
              className="status-dot"
              style="width:18px;height:18px;filter: drop-shadow(0 0 6px rgba(124,255,77,0.6))"
            />
          </span>
          {! authenticated && onLoginSuccess && <Login onSuccess={onLoginSuccess} />}
        </div>
      </div>
    </section>
  );
};
