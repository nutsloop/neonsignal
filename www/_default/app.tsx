import { h, render } from '@nutsloop/neonjsx';

import { Main } from './components/Main';
import { createConsoleStream, TerminalLine } from './scripts/console-stream';
import { createGlitchEffect, createStaticNoise } from './scripts/glitch';

// ASCII box as pre-formatted block
const ASCII_BOX = `
╔══════════════════════════════════════════╗
║  WELCOME TO THE NEON GRID                ║
║  ──────────────────────────────────----  ║
║  "The future is not what it used to be"  ║
║                                          ║
║  SECTOR: ALPHA-7 | ZONE: DEAD STATIC     ║
║  OPERATOR: UNKNOWN                       ║
╚══════════════════════════════════════════╝`;

// Boot sequence messages - synthwave/post-apocalyptic theme
const BOOT_SEQUENCE: TerminalLine[] = [
  { text: '> INITIALIZING WASTELAND RELAY...', type: 'prompt' },
  { text: 'SCANNING FREQUENCIES...', type: 'output' },
  { text: '[OK] UPLINK ESTABLISHED', type: 'success' },
  { text: '', type: 'output' },
  { text: '> RUNNING DIAGNOSTICS...', type: 'prompt' },
  { text: 'RADIATION LEVELS: NOMINAL', type: 'output' },
  { text: 'SIGNAL STRENGTH: 87%', type: 'warning' },
  { text: 'ENCRYPTION: AES-256-NEON', type: 'output' },
  { text: '[OK] SYSTEMS OPERATIONAL', type: 'success' },
  { text: '', type: 'output' },
  { text: '> LOADING TRANSMISSION LOGS...', type: 'prompt' },
  { text: '', type: 'output' },
  { text: 'ASCII_BOX', type: 'output' },
  { text: '', type: 'output' },
  { text: '[TRANSMISSION RELAY ACTIVE]', type: 'success' },
  { text: 'AWAITING INCOMING SIGNALS...', type: 'output' },
];

// Render the app
const App = () => <Main />;
render( <App />, document.getElementById( 'root' )! );

// Initialize effects after render
function initEffects(): void {
  // Get terminal output element
  const terminalOutput = document.getElementById( 'terminal-output' );

  if ( terminalOutput ) {
    // Create console stream
    const stream = createConsoleStream( terminalOutput, {
      speed: 25,
      lineDelay: 150,
    } );

    // Custom streaming that handles the ASCII box specially
    ( async () => {
      for ( const line of BOOT_SEQUENCE ) {
        if ( line.text === 'ASCII_BOX' ) {
          stream.printPre( ASCII_BOX, 'output' );
        }
        else {
          await stream.streamText( line.text, line.type );
        }
        await new Promise( r => setTimeout( r, 150 ) );
      }
      // After boot sequence, set up periodic status messages
      const messages = [
        { text: `[${new Date().toLocaleTimeString()}] SIGNAL PING: OK`, type: 'output' as const },
        { text: `[${new Date().toLocaleTimeString()}] FREQUENCY SCAN COMPLETE`, type: 'success' as const },
        { text: `[${new Date().toLocaleTimeString()}] WARNING: INTERFERENCE DETECTED`, type: 'warning' as const },
      ];
      setInterval( () => {
        const msg = messages[ Math.floor( Math.random() * messages.length ) ];
        stream.printLine( msg.text, msg.type );
        stream.scrollToBottom();
      }, 15000 );
    } )();
  }

  // Register glitch effect on title
  const title = document.querySelector( '.neon-title' );
  if ( title ) {
    const glitch = createGlitchEffect( {
      intensity: 0.3,
      interval: 5000,
      duration: 200,
    } );
    glitch.register( title as HTMLElement );

    // Also trigger glitch on click
    title.addEventListener( 'click', () => {
      glitch.glitchElement( title as HTMLElement );
      const mainContainer = document.querySelector( '.main-container' );
      if ( mainContainer ) {
        glitch.shake( mainContainer as HTMLElement );
      }
    } );
  }

  // Add subtle static noise overlay
  createStaticNoise( document.body, 0.02 );
}

// Wait for DOM to be ready
if ( document.readyState === 'loading' ) {
  document.addEventListener( 'DOMContentLoaded', initEffects );
}
else {
  initEffects();
}
