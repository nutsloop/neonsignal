type HistorySample = { t: number; v: number };

const CPU_KEY = 'neon_cpu_history';
const MEM_KEY = 'neon_mem_history';

const CPU_RETENTION_MS = 24 * 60 * 60 * 1000;
const MEM_RETENTION_MS = 7 * 24 * 60 * 60 * 1000;

const CPU_MAX_POINTS = 20000;
const MEM_MAX_POINTS = 5000;

function loadHistory( key: string ): HistorySample[] {
  if ( typeof sessionStorage === 'undefined' ) {
    return [];
  }
  try {
    const raw = sessionStorage.getItem( key );
    if ( ! raw ) {
      return [];
    }
    const parsed = JSON.parse( raw );
    if ( Array.isArray( parsed ) ) {
      return parsed.filter( item =>
        item && typeof item.t === 'number' && typeof item.v === 'number'
      );
    }

    return [];
  }
  catch {
    return [];
  }
}

function saveHistory( key: string, samples: HistorySample[] ) {
  if ( typeof sessionStorage === 'undefined' ) {
    return;
  }
  try {
    sessionStorage.setItem( key, JSON.stringify( samples ) );
  }
  catch {
    return;
  }
}

function pushSample( key: string, sample: HistorySample, retentionMs: number, maxPoints: number ) {
  const samples = loadHistory( key );
  samples.push( sample );
  const cutoff = Date.now() - retentionMs;
  let start = 0;
  while ( start < samples.length && samples[ start ].t < cutoff ) {
    start += 1;
  }
  const trimmed = samples.slice( start );
  if ( trimmed.length > maxPoints ) {
    trimmed.splice( 0, trimmed.length - maxPoints );
  }
  saveHistory( key, trimmed );
}

export function recordCpuSample( value: number, timestamp = Date.now() ) {
  pushSample( CPU_KEY, { t: timestamp, v: value }, CPU_RETENTION_MS, CPU_MAX_POINTS );
}

export function recordMemSample( value: number, timestamp = Date.now() ) {
  pushSample( MEM_KEY, { t: timestamp, v: value }, MEM_RETENTION_MS, MEM_MAX_POINTS );
}

export function clearHistory() {
  if ( typeof sessionStorage === 'undefined' ) {
    return;
  }
  sessionStorage.removeItem( CPU_KEY );
  sessionStorage.removeItem( MEM_KEY );
}

export function getHistory( kind: 'cpu' | 'mem' ) {
  return loadHistory( kind === 'cpu' ? CPU_KEY : MEM_KEY );
}

export function formatTimestamp( ts: number ) {
  return new Date( ts ).toLocaleString( 'en-GB', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: true,
    weekday: 'short',
  } ).replace( /[,\s]+/g, '_' );
}

function formatCsv( kind: 'cpu' | 'mem', samples: HistorySample[] ) {
  const header = kind === 'cpu'
    ? 'timestamp,cpu_percent\n'
    : 'timestamp,memory_mb\n';

  return header + samples.map( s => `${s.t},${s.v.toFixed( kind === 'cpu' ? 2 : 1 )}` ).join( '\n' );
}

function formatJson( samples: HistorySample[] ) {
  return JSON.stringify( samples, null, 2 );
}

export function exportHistory( kind: 'cpu' | 'mem', format: 'json' | 'csv', rangeMs: number ) {
  const samples = filterRange( getHistory( kind ), rangeMs );
  const body = format === 'json' ? formatJson( samples ) : formatCsv( kind, samples );
  const mime = format === 'json' ? 'application/json' : 'text/csv';

  return { body, mime };
}

export function filterRange( samples: HistorySample[], rangeMs: number ) {
  const cutoff = Date.now() - rangeMs;

  return samples.filter( sample => sample.t >= cutoff );
}

export function renderHistory(
  kind: 'cpu' | 'mem',
  container: HTMLElement,
  rangeMs: number
) {
  const samples = filterRange( getHistory( kind ), rangeMs );
  container.innerHTML = '';
  const strip = document.createElement( 'div' );
  strip.className = 'history-strip';
  const max = kind === 'cpu'
    ? 100
    : Math.max( ...samples.map( s => s.v ), 1 );

  const frag = document.createDocumentFragment();
  samples.forEach( ( sample ) => {
    const bar = document.createElement( 'span' );
    bar.className = kind === 'cpu' ? 'history-bar cpu' : 'history-bar mem';
    const pct = kind === 'cpu'
      ? Math.min( 100, Math.max( 0, sample.v ) )
      : Math.min( 100, ( sample.v / max ) * 100 );
    const h = Math.max( 4, pct );
    bar.style.height = `${h}%`;
    bar.title = `${sample.v.toFixed( kind === 'cpu' ? 2 : 1 )}${kind === 'cpu' ? '%' : ' MB'} @ ${formatTimestamp( sample.t )}`;
    frag.appendChild( bar );
  } );
  strip.appendChild( frag );
  container.appendChild( strip );
}
