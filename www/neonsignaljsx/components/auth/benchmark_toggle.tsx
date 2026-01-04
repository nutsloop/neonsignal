import { getGearMode, setGearMode } from '../../scripts/sse/controller';

const STORAGE_KEY = 'neonsignal_benchmark_mode';
let initialized = false;

function readStoredGearMode() {
  if ( typeof window === 'undefined' ) {
    return 'turbo';
  }
  try {
    const stored = localStorage.getItem( STORAGE_KEY );
    if ( stored === 'kill' || stored === 'throttle' || stored === 'turbo' ) {
      return stored;
    }

    return 'turbo';
  }
  catch {
    return 'turbo';
  }
}

function writeStoredGearMode( mode: string ) {
  if ( typeof window === 'undefined' ) {
    return;
  }
  try {
    localStorage.setItem( STORAGE_KEY, mode );
  }
  catch {
    return;
  }
}

function updateBenchmarkStatus( mode: string ) {
  const status = document.getElementById( 'benchmark-status' );
  let text = 'Live telemetry active.';
  if ( mode === 'throttle' ) {
    text = 'Telemetry throttled for stability.';
  }
  if ( mode === 'kill' ) {
    text = 'Telemetry disabled while benchmarking.';
  }
  if ( status ) {
    status.textContent = text;
    status.classList.toggle( 'active', mode !== 'turbo' );
  }
  document.body.setAttribute( 'data-benchmark', mode );
}

function applyGearMode( mode: 'turbo' | 'throttle' | 'kill' ) {
  setGearMode( mode );
  updateBenchmarkStatus( mode );
  writeStoredGearMode( mode );
}

export function initBenchmarkToggle() {
  if ( initialized ) {
    return;
  }
  initialized = true;
  const stored = readStoredGearMode();
  const inputs = Array.from(
    document.querySelectorAll( 'input[name="benchmark-gear"]' )
  ) as HTMLInputElement[];
  if ( inputs.length === 0 ) {
    return;
  }
  inputs.forEach( ( input ) => {
    input.checked = input.value === stored;
  } );
  applyGearMode( stored as 'turbo' | 'throttle' | 'kill' );
}

export const BenchmarkToggle = () => {
  const onToggle = ( event: Event ) => {
    const target = event.target as HTMLInputElement | null;
    const value = target?.value;
    if ( value === 'turbo' || value === 'throttle' || value === 'kill' ) {
      applyGearMode( value );
    }
  };
  const current = readStoredGearMode() || getGearMode();

  return (
    <section className="panel benchmark">
      <h2>Benchmark Mode</h2>
      <p className="lede">
        Select a gear before running stress tests to keep the console stable.
      </p>
      <div className="gear-row">
        <span className="toggle-label">Telemetry gear</span>
        <div className="gear-toggle" role="radiogroup" aria-label="Telemetry gear">
          <label className="gear-option">
            <input
              type="radio"
              name="benchmark-gear"
              value="throttle"
              onChange={onToggle}
              defaultChecked={current === 'throttle'}
            />
            <span className="gear-pill">Throttle</span>
          </label>
          <label className="gear-option">
            <input
              type="radio"
              name="benchmark-gear"
              value="turbo"
              onChange={onToggle}
              defaultChecked={current === 'turbo'}
            />
            <span className="gear-pill">Turbo</span>
          </label>
          <label className="gear-option">
            <input
              type="radio"
              name="benchmark-gear"
              value="kill"
              onChange={onToggle}
              defaultChecked={current === 'kill'}
            />
            <span className="gear-pill">Kill</span>
          </label>
        </div>
      </div>
      <p id="benchmark-status" className="status" aria-live="polite"></p>
    </section>
  );
};
