import { render } from '@nutsloop/neonjsx';

import { FontShowCase } from './pages/FontShowCase';
import { Main } from './pages/Main';
import { NotFound } from './pages/NotFound';

const App = () => <Main />;
render( <App />, document.getElementById( 'root' )! );

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
    render( <Main />, root );

    return;
  }

  // Codex page (font-show-case)
  if ( neonPath === '/font-showcase.html' ) {

    render( <FontShowCase />, root );

    return;
  }
}

document.addEventListener( 'DOMContentLoaded', () => {
  bootstrap();
} );
