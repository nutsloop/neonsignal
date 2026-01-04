import { registerStreamWithThrottle, shouldProcess } from './controller';

export function sse_redirect_service() {
  try{
    registerStreamWithThrottle( 'redirect', () => {
      const redirectDot = document.getElementById(
        'redirect-dot'
      ) as HTMLImageElement | null;
      const redirectText = document.getElementById( 'redirect-status-text' );
      const esRedirect = new EventSource( '/api/redirect_service' );
      let errorCount = 0;
      const maxErrors = 3;
      esRedirect.onmessage = ( evt ) => {
        if ( ! shouldProcess( 'redirect' ) ) {
          return;
        }
        try {
          const data = JSON.parse( evt.data );
          const ok = !! data.redirect_ok;
          errorCount = 0;
          if ( redirectDot ) {
            redirectDot.src = ok ? '/icons/dot-green.svg' : '/icons/dot-red.svg';
          }
          if ( redirectText ) {
            redirectText.textContent = ok ? 'Redirect OK' : 'Redirect down';
            redirectText.style.color = ok ? 'var(--accent)' : '#ff6b6b';
          }
        }
        catch ( e ) {
          console.error( e );
        }
      };
      esRedirect.onerror = () => {
        errorCount += 1;
        if ( errorCount >= maxErrors ) {
          if ( redirectDot ) {
            redirectDot.src = '/icons/dot-red.svg';
          }
          if ( redirectText ) {
            redirectText.textContent = 'Redirect down';
            redirectText.style.color = '#ff6b6b';
          }
        }
      };

      return esRedirect;
    } );
  }
  catch ( e ) {
    console.error( e );
  }
}
