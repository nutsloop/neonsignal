export const NotFound = ( props: { path: string } ) => (
  <main className="container">
    <div className="logo" aria-hidden="true">
      <h1 className="nutsloop">404</h1>
    </div>
    <p className="coming-soon">Signal lost</p>
    <p className="coming-soon">Missing: {props.path}</p>
  </main>
);
