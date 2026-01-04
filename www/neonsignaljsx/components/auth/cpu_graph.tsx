export const CpuGraph = () => {
  const bars = Array.from( { length: 60 }, ( _, i ) => (
    <div className="cpu-bar" data-index={i} title="…"></div>
  ) );

  return (
    <div className="card cpu-card">
      <div className="card-head">
        <p className="label">CPU (%)</p>
        <p className="value"><span id="cpu-value">…</span></p>
      </div>
      <div className="cpu-graph" id="cpu-graph">
        {bars}
      </div>
    </div>
  );
};
