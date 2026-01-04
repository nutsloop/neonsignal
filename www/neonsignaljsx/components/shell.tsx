export const Shell = ( props: { children?: any } ) => {
  return (
    <div className="wasteland">
      <div className="topbar" data-topbar="true">
        <div className="topbar-inner">
          <span className="topbar-label">Signal Links</span>
          <nav className="topbar-links">
            <a href="https://simonedelpopolo.host" target="_blank" rel="noopener">simonedelpopolo.host</a>
            <a href="https://nutsloop.host" target="_blank" rel="noopener">nutsloop.host</a>
            <a href="https://neonsignal.nutsloop.host" target="_blank" rel="noopener">neonsignal.nutsloop.host</a>
            <a href="https://neonsignal.nutsloop.host/neonsignal-book.html" target="_blank" rel="noopener">book</a>
            <a href="/auth.html">admin</a>
            <a href="/codex.html">codex</a>
          </nav>
        </div>
      </div>
      <div className="sky" aria-hidden="true"></div>
      <div className="grid-floor" aria-hidden="true"></div>
      <div className="scanlines" aria-hidden="true"></div>
      <div className="noise" aria-hidden="true"></div>
      <div className="container">{props.children}</div>
    </div>
  );
};
