import { formatTimestamp, recordCpuSample } from '../graph_history';
import { registerStreamWithThrottle, shouldProcess } from './controller';

export function sse_cpu() {
  try{
    registerStreamWithThrottle( 'cpu', () => {
      const esCpu = new EventSource( '/api/cpu' );
      const cpuHistory: number[] = [];
      const cpuHistoryTs: number[] = [];
      let cpuBucketSum = 0;
      let cpuBucketCount = 0;
      let cpuBucketSecond = Math.floor( Date.now() / 1000 );
      const cpuBars = Array.from( document.querySelectorAll( '#cpu-graph .cpu-bar' ) ) as HTMLElement[];
      let cpuPending = false;
      let cpuLastText = '...';
      const updateCpuBars = () => {
        cpuPending = false;
        const len = cpuBars.length;
        const start = Math.max( 0, cpuHistory.length - len );
        const slice = cpuHistory.slice( start );
        const tsSlice = cpuHistoryTs.slice( start );
        slice.forEach( ( v, idx ) => {
          const bar = cpuBars[ len - slice.length + idx ];
          if ( bar ) {
            const safe = Number.isFinite( v ) ? v : 0;
            let display = safe;
            if ( safe > 0 && safe < 1 ) {
              display = 4 + Math.sqrt( safe ) * 12;
            }
            const h = Math.max( 4, display ); // minimum height
            bar.style.height = `${h}%`;
            bar.style.background = `linear-gradient(180deg, var(--neon-cyan) ${v}%, #1a1030 100%)`;
            const stamp = formatTimestamp( tsSlice[ idx ] ?? Date.now() );
            bar.title = `${v.toFixed( 2 )}% @ ${stamp}`;
          }
        } );
        const val = document.getElementById( 'cpu-value' );
        if ( val ) {
          val.textContent = cpuLastText;
        }
      };

      esCpu.onmessage = ( evt ) => {
        if ( ! shouldProcess( 'cpu' ) ) {
          return;
        }
        try {
          const data = JSON.parse( evt.data );
          const cpu = typeof data.cpu_percent === 'number' ? data.cpu_percent : null;
          if ( cpu !== null ) {
            const clamped = Math.max( 0, Math.min( 100, cpu ) );
            const nowSecond = Math.floor( Date.now() / 1000 );
            if ( nowSecond !== cpuBucketSecond && cpuBucketCount > 0 ) {
              const avg = cpuBucketSum / cpuBucketCount;
              cpuHistory.push( avg );
              cpuHistoryTs.push( Date.now() );
              recordCpuSample( avg );
              if ( cpuHistory.length > 60 ) {
                cpuHistory.shift();
                cpuHistoryTs.shift();
              }
              cpuBucketSum = 0;
              cpuBucketCount = 0;
              cpuBucketSecond = nowSecond;
            }
            cpuBucketSum += clamped;
            cpuBucketCount += 1;
            cpuLastText = `${cpu.toFixed( 2 )}%`;
            if ( ! cpuPending ) {
              cpuPending = true;
              requestAnimationFrame( updateCpuBars );
            }
          }
        }
        catch ( e ) {
          console.error( e );
        }
      };

      return esCpu;
    } );
  }
  catch ( e ) {
    console.error( e );
  }
}
