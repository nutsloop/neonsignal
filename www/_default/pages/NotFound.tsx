import { css } from '@nutsloop/neonjsx';

import { Footer } from '../components/Footer';

export const NotFound = ( props: { path: string } ) => {
  /* fonts */
  css( './css/fonts/press-start-2p.css' );
  css( './css/fonts/audiowide.css' );
  css( './css/fonts/share-tech-mono.css' );
  css( './css/fonts/intel-one-mono.css' );
  /* component styles */
  css( './css/notfound.css' );

  return (
    <div className="notfound-page">
      <div className="notfound">
        <div className="notfound__glitch"></div>
        <div className="notfound__glitch"></div>
        <div className="notfound__glitch"></div>

        <div className="notfound__code">404</div>
        <h1 className="notfound__title">Signal Lost</h1>

        <div className="notfound__panel">
          <p className="notfound__message">
            Resource not found: <code className="notfound__path">{props.path}</code>
          </p>
          <p className="notfound__message">
            HTTP/2 relay could not locate the requested asset.
          </p>
        </div>

        <a href="/" className="notfound__link">← Return to Home</a>
      </div>

      <Footer />
    </div>
  );
};
