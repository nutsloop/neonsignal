import { css } from '@nutsloop/neonjsx';

export const Footer = () => {
  /* fonts */
  css( './css/fonts/orbitron.css' );
  css( './css/fonts/share-tech-mono.css' );
  css( './css/fonts/syncopate.css' );
  /* component styles */
  css( './css/footer.css' );

  return (
    <footer className="site-footer">
      <div className="site-footer__brand">
        <span className="site-footer__logo">skinjo.org</span>
        <p>AI-driven client interaction and wasteland signal briefs.</p>
      </div>
      <div className="site-footer__links">
        <a href="https://neonsignal.nutsloop.com" target="_blank" rel="noreferrer">
          NeonSignal server
        </a>
      </div>
      <div className="site-footer__meta">
        <span>Built for signal clarity.</span>
        <span>Apache 2.0</span>
      </div>
    </footer>
  );
};
