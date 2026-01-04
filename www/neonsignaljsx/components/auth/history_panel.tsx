import { clearHistory, exportHistory, renderHistory } from '../../scripts/graph_history';

const RANGE_OPTIONS = [
  { label: 'Last 1h', value: '1h', ms: 60 * 60 * 1000 },
  { label: 'Last 6h', value: '6h', ms: 6 * 60 * 60 * 1000 },
  { label: 'Last 24h', value: '24h', ms: 24 * 60 * 60 * 1000 },
  { label: 'Last 7d', value: '7d', ms: 7 * 24 * 60 * 60 * 1000 },
];

function downloadFile( name: string, body: string, mime: string ) {
  const blob = new Blob( [ body ], { type: mime } );
  const url = URL.createObjectURL( blob );
  const link = document.createElement( 'a' );
  link.href = url;
  link.download = name;
  document.body.appendChild( link );
  link.click();
  link.remove();
  window.setTimeout( () => URL.revokeObjectURL( url ), 1000 );
}

function getRangeMs() {
  const select = document.getElementById( 'history-range' ) as HTMLSelectElement | null;
  const selected = select?.value || '1h';
  const entry = RANGE_OPTIONS.find( option => option.value === selected );

  return entry ? entry.ms : RANGE_OPTIONS[ 0 ].ms;
}

function refreshHistory() {
  const cpuContainer = document.getElementById( 'cpu-history-graph' );
  const memContainer = document.getElementById( 'mem-history-graph' );
  const rangeMs = getRangeMs();
  if ( cpuContainer ) {
    renderHistory( 'cpu', cpuContainer, rangeMs );
  }
  if ( memContainer ) {
    renderHistory( 'mem', memContainer, rangeMs );
  }
}

export function initHistoryPanel() {
  const openBtn = document.getElementById( 'history-open' );
  const closeBtn = document.getElementById( 'history-close' );
  const overlay = document.getElementById( 'history-overlay' );
  const rangeSelect = document.getElementById( 'history-range' );
  const clearBtn = document.getElementById( 'history-clear' );
  const exportJsonBtn = document.getElementById( 'history-export-json' );
  const exportCsvBtn = document.getElementById( 'history-export-csv' );

  if ( ! openBtn || ! closeBtn || ! overlay ) {
    return;
  }

  const openOverlay = () => {
    overlay.classList.add( 'open' );
    refreshHistory();
  };

  const closeOverlay = () => {
    overlay.classList.remove( 'open' );
  };

  openBtn.addEventListener( 'click', openOverlay );
  closeBtn.addEventListener( 'click', closeOverlay );
  overlay.addEventListener( 'click', ( event ) => {
    if ( event.target === overlay ) {
      closeOverlay();
    }
  } );

  rangeSelect?.addEventListener( 'change', refreshHistory );
  clearBtn?.addEventListener( 'click', () => {
    clearHistory();
    refreshHistory();
  } );
  exportJsonBtn?.addEventListener( 'click', () => {
    const rangeMs = getRangeMs();
    const cpu = exportHistory( 'cpu', 'json', rangeMs );
    const mem = exportHistory( 'mem', 'json', rangeMs );
    downloadFile( 'neonsignal_cpu_history.json', cpu.body, cpu.mime );
    window.setTimeout( () => {
      downloadFile( 'neonsignal_memory_history.json', mem.body, mem.mime );
    }, 150 );
  } );
  exportCsvBtn?.addEventListener( 'click', () => {
    const rangeMs = getRangeMs();
    const cpu = exportHistory( 'cpu', 'csv', rangeMs );
    const mem = exportHistory( 'mem', 'csv', rangeMs );
    downloadFile( 'neonsignal_cpu_history.csv', cpu.body, cpu.mime );
    window.setTimeout( () => {
      downloadFile( 'neonsignal_memory_history.csv', mem.body, mem.mime );
    }, 150 );
  } );
}

export const HistoryPanel = () => (
  <div id="history-overlay" className="history-overlay" role="dialog" aria-modal="true">
    <div className="history-modal">
      <div className="history-header">
        <h2>History Vault</h2>
        <button id="history-close" className="pill">Close</button>
      </div>
      <p className="lede">
        Stored in session only. Close the browser to wipe the archive.
      </p>
      <div className="history-controls">
        <label className="history-label">
          Range
          <select id="history-range" className="history-select">
            {RANGE_OPTIONS.map( option => (
              <option value={option.value} key={option.value}>{option.label}</option>
            ) )}
          </select>
        </label>
        <div className="history-actions">
          <button id="history-export-json" className="pill">Export JSON</button>
          <button id="history-export-csv" className="pill">Export CSV</button>
          <button id="history-clear" className="pill danger">Clear</button>
        </div>
      </div>
      <div className="history-section">
        <h3>CPU Archive</h3>
        <div id="cpu-history-graph" className="history-graph"></div>
      </div>
      <div className="history-section">
        <h3>Memory Archive</h3>
        <div id="mem-history-graph" className="history-graph"></div>
      </div>
    </div>
  </div>
);
