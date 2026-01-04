import { registerStreamWithThrottle, shouldProcess } from './controller';

export function sse_events() {
  try {
    registerStreamWithThrottle( 'events', () => {
      const es = new EventSource( '/api/events' );
      es.onmessage = ( evt ) => {
        if ( ! shouldProcess( 'events' ) ) {
          return;
        }
        try {
          const data = JSON.parse( evt.data );
          const el = document.getElementById( 'files-served-value' );
          if ( el && typeof data.files_served !== 'undefined' ) {
            el.textContent = String( data.files_served );
          }
          const pv = document.getElementById( 'page-views-value' );
          if ( pv && typeof data.page_views !== 'undefined' ) {
            pv.textContent = String( data.page_views );
          }
        }
        catch ( e ) {
          console.error( e );
        }
      };

      return es;
    } );
  }
  catch ( e ) {
    console.error( e );
  }
}
