import { render } from '../../neonjsx/runtime';
import { AboutPage } from './about';
import { AuthApp } from './components/auth/auth_app';
// import { DataPage } from './components/auth/data';
import { Codex } from './components/auth/codex';
import { NotFound } from './components/notfound';
import { Shell } from './components/shell';
import { LandingPage } from './landing';

function isAuthenticated(): boolean {
  try {
    const hasSession = document.cookie
      .split( ';' )
      .some( c => c.trim().startsWith( 'ns_session=' ) || c.trim().startsWith( 'ns_debug=' ) );
    if ( hasSession ) {
      return true;
    }
    const stored = localStorage.getItem( 'ns_auth_result' );

    return stored === 'ok';
  }
  catch ( e ) {

    console.error( e );

    return false;
  }
}

function bootstrap() {
  const root = document.getElementById( 'root' );
  if ( ! root ) {
    return;
  }
  const neonStatus = ( window as any ).__NEON_STATUS;
  const neonPath = ( window as any ).__NEON_PATH || window.location.pathname;

  if ( neonStatus === 404 ) {
    render( <Shell><NotFound path={neonPath} /></Shell>, root );

    return;
  }

  // Landing page at / and /index.html
  if ( neonPath === '/' || neonPath === '/index.html' ) {
    render( <Shell><LandingPage /></Shell>, root );

    return;
  }

  // Codex page (auth-only)
  if ( neonPath === '/codex.html' ) {
    const authed = isAuthenticated();
    if ( ! authed ) {
      render( <Shell><AuthApp authenticated={false} /></Shell>, root );

      return;
    }
    render( <Shell><Codex /></Shell>, root );

    return;
  }

  if ( neonPath === '/about.html' ) {
    render( <Shell><AboutPage /></Shell>, root );

    return;
  }


  const authed = isAuthenticated();
  // Unknown path - show landing
  render( <Shell><AuthApp authenticated={authed} /></Shell>, root );

  // After the UI is rendered, surface the last auth result (even across reloads).
  try {
    const last = localStorage.getItem( 'ns_auth_result' );
    if ( last ) {
      const statusEl = document.getElementById( 'auth-status' );
      if ( statusEl ) {
        statusEl.textContent = `Last auth: ${last}`;
      }
      localStorage.removeItem( 'ns_auth_result' );
    }
  }
  catch ( e ) {
    console.error( e );
  }
}

document.addEventListener( 'DOMContentLoaded', () => {
  bootstrap();
} );
