const STORAGE_KEY = 'neonsignal_visual_mode';
let initialized = false;

function readStoredVisualMode() {
  if ( typeof window === 'undefined' ) {
    return 'full';
  }
  try {
    return localStorage.getItem( STORAGE_KEY ) === 'low' ? 'low' : 'full';
  }
  catch {
    return 'full';
  }
}

function writeStoredVisualMode( mode: string ) {
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

function applyVisualMode( mode: 'full' | 'low' ) {
  document.body.setAttribute( 'data-visual', mode );
  const status = document.getElementById( 'visual-status' );
  if ( status ) {
    status.textContent = mode === 'low'
      ? 'Visual load reduced for stability.'
      : 'Full synthwave visuals enabled.';
    status.classList.toggle( 'active', mode === 'low' );
  }
  writeStoredVisualMode( mode );
}

export function initVisualToggle() {
  if ( initialized ) {
    return;
  }
  initialized = true;
  const input = document.getElementById( 'visual-toggle' ) as HTMLInputElement | null;
  if ( ! input ) {
    return;
  }
  const stored = readStoredVisualMode();
  input.checked = stored === 'low';
  applyVisualMode( stored as 'full' | 'low' );
}

export const VisualToggle = () => {
  const stored = readStoredVisualMode();
  const onToggle = ( event: Event ) => {
    const target = event.target as HTMLInputElement | null;
    const enabled = !! target?.checked;
    applyVisualMode( enabled ? 'low' : 'full' );
  };

  return (
    <section className="panel visual">
      <h2>Sight Mode</h2>
      <p className="lede">
        Dial back the glow when browsers start to sweat.
      </p>
      <div className="toggle-row">
        <span className="toggle-label">Visual load</span>
        <label className="switch">
          <input
            id="visual-toggle"
            type="checkbox"
            defaultChecked={stored === 'low'}
            onChange={onToggle}
            aria-label="Visual performance mode"
          />
          <span className="switch-track" aria-hidden="true">
            <span className="switch-knob"></span>
          </span>
        </label>
      </div>
      <p id="visual-status" className="status" aria-live="polite"></p>
    </section>
  );
};
