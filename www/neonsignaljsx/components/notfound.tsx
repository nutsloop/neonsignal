export const NotFound = ( props: { path: string } ) => (
  <div className="page">
    <section className="panel stats">
      <h2>404 · Signal Lost</h2>
      <p>Resource not found: <code>{props.path}</code></p>
      <p>HTTP/2 relay could not locate the requested asset.</p>
    </section>
  </div>
);
