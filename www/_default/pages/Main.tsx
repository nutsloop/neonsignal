import { css } from '@nutsloop/neonjsx';

import { Footer } from '../components/Footer';

export const Main = () => {
  /* fonts */
  css( './css/fonts/orbitron.css' );
  css( './css/fonts/share-tech-mono.css' );
  css( './css/fonts/syncopate.css' );
  css( './css/fonts/audiowide.css' );
  /* component styles */
  css( './css/main.css' );

  return (
    <>
      <main className="main-landing">
        <div className="main-landing__glow"></div>
        <div className="main-landing__grid"></div>

        <header className="main-landing__header">
          <a className="main-landing__brand" href="/">skinjo.org</a>
          <a
            className="main-landing__nav-link"
            href="https://neonsignal.nutsloop.com"
            target="_blank"
            rel="noreferrer"
          >
            NeonSignal server
          </a>
        </header>

        <section className="main-landing__hero">
          <div className="main-landing__hero-copy">
            <p className="main-landing__eyebrow">Wasteland signal dispatch</p>
            <h1 className="main-landing__title">
              skinjo.org AI relay, forged in the wasteland grid.
            </h1>
            <p className="main-landing__lede">
              skinjo is a Blade Runner skinjob reference: a neon-dusted outpost
              for AI-guided client interaction, blog-style insights, and signal
              summaries. Keywords: AI concierge, AI assistant, AI interaction,
              cyberpunk wasteland, synthwave, HTTP/2, TLS, realtime routing,
              edge compute, low latency, inference pipeline.
            </p>
            <div className="main-landing__actions">
              <a
                className="main-landing__button main-landing__button--solid"
                href="https://neonsignal.nutsloop.com"
                target="_blank"
                rel="noreferrer"
              >
                Visit NeonSignal
              </a>
            </div>
          </div>

          <div className="main-landing__hero-panels">
            <div className="main-landing__panel main-landing__panel--alpha">
              <p className="main-landing__panel-label">Skinjo signal</p>
              <h3>AI client relay</h3>
              <p>AI-guided intake, summaries, and intent mapping for new sessions.</p>
            </div>
            <div className="main-landing__panel main-landing__panel--beta">
              <p className="main-landing__panel-label">Wasteland brief</p>
              <h3>Signal blog keywords</h3>
              <p>Short, sharp posts built from AI notes and client context.</p>
            </div>
            <div className="main-landing__panel main-landing__panel--gamma">
              <p className="main-landing__panel-label">NeonSignal</p>
              <h3>HTTP/2 backbone</h3>
              <p>Powered by the NeonSignal server stack. Built for speed and clarity.</p>
            </div>
          </div>
        </section>

        <section className="main-landing__signal">
          <div className="main-landing__signal-header">
            <p className="main-landing__eyebrow">Wasteland brief</p>
            <h2>AI interaction map</h2>
            <p>
              From first contact to session summary, the relay distills intent
              into clear, searchable traces. Every encounter feeds the signal log.
            </p>
          </div>
          <div className="main-landing__signal-grid">
            <div className="main-landing__signal-card">
              <h4>Contact</h4>
              <p>AI greets, captures intent, and routes inquiries to the right node.</p>
              <span>AI concierge</span>
            </div>
            <div className="main-landing__signal-card">
              <h4>Summarize</h4>
              <p>Concise briefs with keywords, tags, and action context.</p>
              <span>AI summary</span>
            </div>
            <div className="main-landing__signal-card">
              <h4>Publish</h4>
              <p>Blog-style dispatches built from the log, ready for clients.</p>
              <span>AI keywords</span>
            </div>
          </div>
        </section>

        <section className="main-landing__metrics">
          <div className="main-landing__metric">
            <p className="main-landing__metric-value">AI</p>
            <p className="main-landing__metric-label">Client relay</p>
          </div>
          <div className="main-landing__metric">
            <p className="main-landing__metric-value">Wasteland</p>
            <p className="main-landing__metric-label">Signal log</p>
          </div>
          <div className="main-landing__metric">
            <p className="main-landing__metric-value">Keywords</p>
            <p className="main-landing__metric-label">Blog briefs</p>
          </div>
          <div className="main-landing__metric">
            <p className="main-landing__metric-value">NeonSignal</p>
            <p className="main-landing__metric-label">Server stack</p>
          </div>
        </section>
      </main>

      <Footer />
    </>
  );
};
