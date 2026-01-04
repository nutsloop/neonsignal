export const Timeline = () => (
  <section className="panel timeline">
    <h2>Boot Sequence</h2>
    <ol>
      <li>
                ALPN negotiates <code>h2</code> and TLS 1.2+
      </li>
      <li>Preface validated; SETTINGS + ACK exchanged</li>
      <li>HPACK headers parsed with dynamic table support</li>
      <li>
                Static router resolves <code>public/</code> assets
      </li>
      <li>Epoll drives IO; threads handle work without blocking</li>
    </ol>
  </section>
);