import { Fragment } from '@nutsloop/neonjsx';

import { CpuGraph } from './cpu_graph';
import { HistoryPanel } from './history_panel';
import { MemoryBlocks } from './memory_blocks';

type Stat = { label: string; value: string; accent?: boolean };

const stats: Stat[] = [
  { label: 'Protocol', value: 'HTTP/2 only', accent: true },
  { label: 'TLS', value: 'OpenSSL, ALPN h2' },
  { label: 'HPACK', value: 'nghttp2 inflater (no custom tables)' },
  { label: 'Concurrency', value: 'Epoll + multithread handshake' },
  { label: 'Static', value: 'Public assets via router' },
  { label: 'Preface', value: 'Validated, SETTINGS ACK' }
];

type StatsProps = { authenticated: boolean };

export const Stats = ( { authenticated }: StatsProps ) => (
  <Fragment>
    {authenticated ? (
      <Fragment>
        <section className="panel stats">
          <div className="panel-head">
            <h2>Server Status</h2>
          </div>
          <div className="grid">
            {stats.map( s => (
              <div className={`card ${s.accent ? 'accent' : ''}`} key={s.label}>
                <p className="label">{s.label}</p>
                <p className="value">{s.value}</p>
              </div>
            ) )}
            <div className="card" key="files-served">
              <p className="label">Files served</p>
              <p className="value">
                <span id="files-served-value">…</span>
              </p>
            </div>
            <div className="card" key="page-views">
              <p className="label">Page views</p>
              <p className="value">
                <span id="page-views-value">…</span>
              </p>
            </div>
            <div className="card" key="redirect">
              <p className="label">Redirector (http(80) → https(443))</p>
              <div className="status-row">
                <img
                  id="redirect-dot"
                  className="status-dot"
                  src="/icons/dot-red.svg"
                  alt="redirect service status"
                />
                <span id="redirect-status-text" className="status-text">
                Checking…
                </span>
              </div>
            </div>
          </div>
          <div className="graph-row">
            <CpuGraph />
          </div>
          <div className="graph-row">
            <MemoryBlocks />
          </div>
          <div className="history-cta">
            <p className="hint">
            History archives CPU and memory graphs for this session only.
            </p>
            <button id="history-open" className="pill">Open History</button>
          </div>
        </section>
        <HistoryPanel />
      </Fragment>
    ) : (
      <Fragment />
    )}
  </Fragment>
);
