export const MemoryBlocks = () => {
  const bars = Array.from( { length: 60 }, ( _, i ) => (
    <div className="mem-bar" data-index={i} title="…"></div>
  ) );

  return (
    <div className="card mem-card">
      <div className="card-head">
        <p className="label">RSS (MB)</p>
        <p className="value"><span id="mem-value">…</span></p>
      </div>
      <div className="mem-graph" id="mem-graph">
        {bars}
      </div>
    </div>
  );
};
