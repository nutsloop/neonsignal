import { formatTimestamp, recordMemSample } from '../graph_history';
import { registerStreamWithThrottle, shouldProcess } from './controller';

export function sse_memory(){

  try{
    registerStreamWithThrottle( 'memory', () => {
      const esMem = new EventSource( '/api/memory' );
      const memHistory: number[] = [];
      const memHistoryTs: number[] = [];
      const memBars = Array.from( document.querySelectorAll( '#mem-graph .mem-bar' ) ) as HTMLElement[];
      let memPending = false;
      let memLastText = '...';
      const updateMemBars = () => {
        memPending = false;
        const len = memBars.length;
        const start = Math.max( 0, memHistory.length - len );
        const slice = memHistory.slice( start );
        const tsSlice = memHistoryTs.slice( start );
        const max = Math.max( ...slice, 1 );
        slice.forEach( ( v, idx ) => {
          const bar = memBars[ len - slice.length + idx ];
          if ( bar ) {
            const pct = Math.min( 100, ( v / max ) * 100 );
            const safePct = Number.isFinite( pct ) ? pct : 0;
            const h = Math.max( 4, safePct ); // minimum height
            bar.style.height = `${h}%`;
            bar.style.background = `linear-gradient(180deg, var(--neon-pink) ${pct}%, #0f0a1c 100%)`;
            const stamp = formatTimestamp( tsSlice[ idx ] ?? Date.now() );
            bar.title = `${v.toFixed( 1 )} MB @ ${stamp}`;
          }
        } );
        const val = document.getElementById( 'mem-value' );
        if ( val ) {
          val.textContent = memLastText;
        }
      };

      esMem.onmessage = ( evt ) => {
        if ( ! shouldProcess( 'memory' ) ) {
          return;
        }
        try {
          const data = JSON.parse( evt.data );
          const rss = typeof data.rss_kb === 'number' ? data.rss_kb : null;
          if ( rss !== null ) {
            const rssMb = rss / 1024;
            memLastText = `${rssMb.toFixed( 1 )} MB`;
            memHistory.push( rssMb );
            memHistoryTs.push( Date.now() );
            recordMemSample( rssMb );
            if ( memHistory.length > 60 ) {
              memHistory.shift();
              memHistoryTs.shift();
            }
            if ( ! memPending ) {
              memPending = true;
              requestAnimationFrame( updateMemBars );
            }
          }
        }
        catch ( e ) {
          console.error( e );
        }
      };

      return esMem;
    } );
  }
  catch ( e ) {
    console.error( e );
  }
}
