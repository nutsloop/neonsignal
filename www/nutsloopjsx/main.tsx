import { render } from '@nutsloop/neonjsx';

import { NotFound } from './components/notfound';
import { LandingPage } from './landing';

function bootstrap() {
  const root = document.getElementById( 'root' );
  if ( ! root ) {
    return;
  }
  const neonStatus = ( window as any ).__NEON_STATUS;
  const neonPath = ( window as any ).__NEON_PATH || window.location.pathname;

  if ( neonStatus === 404 ) {
    render( <NotFound path={neonPath} />, root );

    return;
  }

  // Landing page at / and /index.html
  if ( neonPath === '/' || neonPath === '/index.html' ) {
    render( <LandingPage />, root );

    return;
  }
}

document.addEventListener( 'DOMContentLoaded', bootstrap );
