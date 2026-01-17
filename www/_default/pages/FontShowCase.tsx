import { css } from '@nutsloop/neonjsx';

import { Footer } from '../components/Footer';

export const FontShowCase = () => {
  /* fonts */
  css( './css/fonts/orbitron.css' );
  css( './css/fonts/share-tech-mono.css' );
  css( './css/fonts/intel-one-mono.css' );
  css( './css/fonts/monoton.css' );
  css( './css/fonts/audiowide.css' );
  css( './css/fonts/syncopate.css' );
  css( './css/fonts/press-start-2p.css' );
  css( './css/fonts/bungee.css' );
  /* component styles */
  css( './css/font-showcase.css' );

  return (
    <div className="font-showcase-page">
      <div className="font-showcase">
        <h1 className="title">Synthwave Font Palette</h1>

        <div className="font-sample orbitron">
          <span className="label">Orbitron</span>
          <span className="preview">NEON SIGNAL 2084</span>
        </div>

        <div className="font-sample share-tech">
          <span className="label">Share Tech Mono</span>
          <span className="preview">system.init() → ready</span>
        </div>

        <div className="font-sample intel">
          <span className="label">Intel One Mono</span>
          <span className="preview">const signal = await connect();</span>
        </div>

        <div className="font-sample monoton">
          <span className="label">Monoton</span>
          <span className="preview">NEON</span>
        </div>

        <div className="font-sample audiowide">
          <span className="label">Audiowide</span>
          <span className="preview">RETRO ARCADE 1985</span>
        </div>

        <div className="font-sample syncopate">
          <span className="label">Syncopate</span>
          <span className="preview">WIDE TRANSMISSION</span>
        </div>

        <div className="font-sample press-start">
          <span className="label">Press Start 2P</span>
          <span className="preview">GAME OVER</span>
        </div>

        <div className="font-sample bungee">
          <span className="label">Bungee</span>
          <span className="preview">IMPACT</span>
        </div>
      </div>

      <Footer />
    </div>
  );
};
