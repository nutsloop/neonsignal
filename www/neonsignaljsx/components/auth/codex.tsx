export const initCodexForm = () => {
  const form = document.querySelector( '[data-codex-form="true"]' ) as HTMLFormElement | null;
  const status = document.querySelector( '.codex-status' ) as HTMLElement | null;
  const listEl = document.querySelector( '[data-codex-list]' ) as HTMLElement | null;
  const detailEl = document.querySelector( '[data-codex-detail]' ) as HTMLElement | null;
  const idInput = document.querySelector( '[data-codex-id]' ) as HTMLInputElement | null;
  const refreshButton = document.querySelector( '[data-codex-refresh]' ) as HTMLButtonElement | null;
  const loadButton = document.querySelector( '[data-codex-load]' ) as HTMLButtonElement | null;
  const runButton = document.querySelector( '[data-codex-run]' ) as HTMLButtonElement | null;
  const runLoadButton = document.querySelector( '[data-codex-run-load]' ) as HTMLButtonElement | null;
  const runIdInput = document.querySelector( '[data-codex-run-id]' ) as HTMLInputElement | null;
  const runStatus = document.querySelector( '[data-codex-run-status]' ) as HTMLElement | null;
  const runMeta = document.querySelector( '[data-codex-run-meta]' ) as HTMLElement | null;
  const runStdout = document.querySelector( '[data-codex-run-stdout]' ) as HTMLElement | null;
  const runStderr = document.querySelector( '[data-codex-run-stderr]' ) as HTMLElement | null;
  const runArtifacts = document.querySelector( '[data-codex-run-artifacts]' ) as HTMLElement | null;
  if ( ! form || form.dataset.codexInit === 'true' ) {
    return;
  }
  form.dataset.codexInit = 'true';

  const renderList = ( items: any[] ) => {
    if ( ! listEl ) {
      return;
    }
    if ( ! items.length ) {
      listEl.textContent = 'No codex entries yet.';

      return;
    }
    listEl.innerHTML = items
      .map( ( item ) => {
        const id = item.id || item.codex_id || 'unknown';
        const title = item.title || 'Untitled';
        const created = item.created_at ? new Date( item.created_at * 1000 ).toISOString() : '';

        return `<button type="button" class="pill ghost" data-codex-item="${id}">
          <span>${title}</span>
          <span>${id}${created ? ` · ${created}` : ''}</span>
        </button>`;
      } )
      .join( '' );
  };

  const previewEl = document.querySelector( '[data-codex-preview]' ) as HTMLElement | null;

  const renderDetail = ( payload: any ) => {
    if ( ! detailEl ) {
      return;
    }
    if ( ! payload ) {
      detailEl.textContent = 'No detail available.';
      if ( previewEl ) {
        previewEl.textContent = '';
      }

      return;
    }
    detailEl.textContent = JSON.stringify( payload, null, 2 );
    if ( ! previewEl ) {
      return;
    }
    previewEl.textContent = '';
    if ( payload.image_size && payload.id ) {
      const img = document.createElement( 'img' );
      img.alt = payload.image_alt || 'Codex image';
      img.src = `/api/codex/image?id=${encodeURIComponent( payload.id )}`;
      const caption = document.createElement( 'div' );
      caption.className = 'codex-preview-meta';
      caption.textContent = payload.image_name
        ? `${payload.image_name} · ${payload.image_size} bytes`
        : `${payload.image_size} bytes`;
      previewEl.appendChild( img );
      previewEl.appendChild( caption );
    }
  };

  const refreshList = async () => {
    if ( ! listEl ) {
      return;
    }
    listEl.textContent = 'Loading entries...';
    try {
      const res = await fetch( '/api/codex/list', { method: 'GET' } );
      const payload = await res.json().catch( () => null );
      const items = payload?.items || payload?.entries || [];
      renderList( Array.isArray( items ) ? items : [] );
    }
    catch ( _err ) {
      listEl.textContent = 'Unable to load codex entries.';
    }
  };



  const loadDetail = async ( id: string ) => {
    if ( ! detailEl || ! id ) {
      return;
    }
    detailEl.textContent = 'Loading detail...';
    try {
      const res = await fetch( `/api/codex/item?id=${encodeURIComponent( id )}`, { method: 'GET' } );
      const payload = await res.json().catch( () => null );
      renderDetail( payload );
    }
    catch ( _err ) {
      if ( detailEl ) {
        detailEl.textContent = 'Unable to load detail.';
      }
    }
  };

  const startRun = async ( id: string ) => {
    if ( ! id ) {
      if ( runStatus ) {
        runStatus.textContent = 'Select a Codex entry before running.';
      }

      return;
    }
    if ( runStatus ) {
      runStatus.textContent = 'Starting Codex run...';
    }
    try {
      const res = await fetch( `/api/codex/run?id=${encodeURIComponent( id )}`, { method: 'POST' } );
      const payload = await res.json().catch( () => null );
      if ( ! res.ok ) {
        const msg = payload?.error ? payload.error : 'Run start failed';
        if ( runStatus ) {
          runStatus.textContent = `Codex run error: ${msg}`;
        }

        return;
      }
      const runId = payload?.run_id || '';
      if ( runIdInput ) {
        runIdInput.value = runId;
      }
      if ( runStatus ) {
        runStatus.textContent = `Run started: ${runId}`;
      }
      loadRun( runId );
    }
    catch ( _err ) {
      if ( runStatus ) {
        runStatus.textContent = 'Codex run error: network failure';
      }
    }
  };

  const loadRun = async ( id: string ) => {
    if ( ! id ) {
      return;
    }
    if ( runMeta ) {
      runMeta.textContent = 'Loading run status...';
    }
    try {
      const res = await fetch( `/api/codex/run/status?id=${encodeURIComponent( id )}` );
      const payload = await res.json().catch( () => null );
      if ( ! res.ok ) {
        const msg = payload?.error ? payload.error : 'Unable to load run';
        if ( runMeta ) {
          runMeta.textContent = `Run error: ${msg}`;
        }

        return;
      }
      if ( runMeta ) {
        runMeta.textContent = JSON.stringify( payload, null, 2 );
      }
      const stdoutRes = await fetch( `/api/codex/run/stdout?id=${encodeURIComponent( id )}` );
      if ( runStdout ) {
        runStdout.textContent = stdoutRes.ok ? await stdoutRes.text() : 'No stdout available.';
      }
      const stderrRes = await fetch( `/api/codex/run/stderr?id=${encodeURIComponent( id )}` );
      if ( runStderr ) {
        runStderr.textContent = stderrRes.ok ? await stderrRes.text() : 'No stderr available.';
      }
      const artifactsRes = await fetch( `/api/codex/run/artifacts?id=${encodeURIComponent( id )}` );
      if ( runArtifacts ) {
        runArtifacts.textContent = artifactsRes.ok ? await artifactsRes.text()
          : 'No artifacts available.';
      }
    }
    catch ( _err ) {
      if ( runMeta ) {
        runMeta.textContent = 'Unable to load run status.';
      }
    }
  };

  refreshButton?.addEventListener( 'click', () => {
    refreshList();
  } );

  loadButton?.addEventListener( 'click', () => {
    loadDetail( idInput?.value || '' );
  } );

  runButton?.addEventListener( 'click', () => {
    startRun( idInput?.value || '' );
  } );

  runLoadButton?.addEventListener( 'click', () => {
    loadRun( runIdInput?.value || '' );
  } );

  listEl?.addEventListener( 'click', ( event ) => {
    const target = event.target as HTMLElement | null;
    const button = target?.closest( '[data-codex-item]' ) as HTMLElement | null;
    if ( ! button ) {
      return;
    }
    const id = button.getAttribute( 'data-codex-item' ) || '';
    if ( idInput ) {
      idInput.value = id;
    }
    loadDetail( id );
  } );

  form.addEventListener( 'submit', async ( event ) => {
    event.preventDefault();
    if ( status ) {
      status.textContent = 'Queueing brief...';
    }
    const body = new FormData( form );
    try {
      const res = await fetch( '/api/codex/brief', {
        method: 'POST',
        body,
      } );
      const payload = await res.json().catch( () => null );
      if ( ! res.ok ) {
        const msg = payload?.error ? payload.error : 'Request failed';
        if ( status ) {
          status.textContent = `Codex error: ${msg}`;
        }

        return;
      }
      if ( status ) {
        const id = payload?.codex_id || 'unknown';
        const bytes = payload?.bytes || 0;
        const verified = payload?.verified ? 'true' : 'false';
        const msg = payload?.message || 'Codex queued.';
        status.textContent = `Queued ${id} · ${bytes} bytes · verified=${verified}\n${msg}`;
      }
      refreshList();
    }
    catch ( _err ) {
      if ( status ) {
        status.textContent = 'Codex error: network failure';
      }
    }
  } );

  refreshList();
};

export const Codex = () => {
  queueMicrotask( initCodexForm );

  return (
    <section className="panel codex-panel">
      <h2>Codex Console</h2>
      <p className="lede">
        Draft an agent-ready brief with wasteland-grade clarity.
      </p>
      <form className="codex-grid" data-codex-form="true">
        <label>
          Title
          <input type="text" name="title" placeholder="Signal name" required />
        </label>
        <label>
          SEO Meta Tags
          <input
            type="text"
            name="meta"
            placeholder="neonsignal, http2, wasteland server"
            required
          />
          <span className="codex-help">Comma-separated keywords for SEO.</span>
        </label>
        <label className="full">
          Description to the Agent
          <textarea
            name="description"
            placeholder="Explain the objective, constraints, and expected output."
            rows={5}
            required
          ></textarea>
        </label>
        <label>
          Image Upload
          <input type="file" name="image" accept="image/*" />
          <span className="codex-help">Optional. Image will be attached to the request.</span>
        </label>
        <label>
          Image Alt
          <input
            type="text"
            name="imageAlt"
            placeholder="Describe the upload for accessibility"
          />
          <span className="codex-help">Optional.</span>
        </label>
        <label>
          Image Metadata
          <input
            type="text"
            name="imageMeta"
            placeholder="Camera, style, or reference notes"
          />
          <span className="codex-help">Optional.</span>
        </label>
        <label className="full">
          File References (codex-cli array)
          <textarea
            name="fileRefs"
            placeholder="One file per line, workspace-relative."
            rows={4}
            required
          ></textarea>
          <span className="codex-help">
            Example: <code>src/neonsignal/http2_listener.c++</code>
          </span>
        </label>
        <div className="codex-actions">
          <button type="submit" className="pill primary">Queue Brief</button>
          <button type="reset" className="pill">Reset</button>
        </div>
        <div className="codex-status" aria-live="polite">
          All required fields must be filled before submission.
        </div>
      </form>
      <div className="codex-history">
        <div className="codex-actions">
          <button type="button" className="pill" data-codex-refresh="true">Refresh Entries</button>
          <input
            type="text"
            className="pill"
            placeholder="Codex ID"
            data-codex-id="true"
          />
          <button type="button" className="pill primary" data-codex-load="true">Load Detail</button>
        </div>
        <div className="codex-actions">
          <button type="button" className="pill" data-codex-run="true">Run Codex</button>
          <input
            type="text"
            className="pill"
            placeholder="Run ID"
            data-codex-run-id="true"
          />
          <button type="button" className="pill primary" data-codex-run-load="true">
            Load Run
          </button>
        </div>
        <div className="codex-run-status" data-codex-run-status="true" aria-live="polite">
          No codex runs yet.
        </div>
        <div className="codex-list" data-codex-list="true">
          No codex entries yet.
        </div>
        <div className="codex-preview" data-codex-preview="true"></div>
        <pre className="codex-detail" data-codex-detail="true"></pre>
        <div className="codex-run-output">
          <pre className="codex-run-meta" data-codex-run-meta="true"></pre>
          <pre className="codex-run-stdout" data-codex-run-stdout="true"></pre>
          <pre className="codex-run-stderr" data-codex-run-stderr="true"></pre>
          <pre className="codex-run-artifacts" data-codex-run-artifacts="true"></pre>
        </div>
      </div>
    </section>
  );
};
