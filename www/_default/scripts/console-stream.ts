/**
 * ConsoleStream - Typewriter-style text streaming for terminal effects
 * Separation of Concerns: This module handles ONLY the streaming logic
 */

export interface StreamOptions {
  speed?: number; // ms per character
  lineDelay?: number; // ms between lines
  cursorChar?: string; // cursor display character
  onChar?: ( char: string ) => void;
  onLine?: ( line: string ) => void;
  onComplete?: () => void;
}

export interface TerminalLine {
  text: string;
  type?: 'prompt' | 'command' | 'output' | 'error' | 'warning' | 'success';
}

const DEFAULT_OPTIONS: StreamOptions = {
  speed: 35,
  lineDelay: 400,
  cursorChar: '█',
};

export class ConsoleStream {
  private element: HTMLElement | null = null;
  private cursorElement: HTMLElement | null = null;
  private options: StreamOptions;
  private isStreaming = false;
  private abortController: AbortController | null = null;

  constructor( options: Partial<StreamOptions> = {} ) {
    this.options = { ...DEFAULT_OPTIONS, ...options };
  }

  /**
   * Attach to a DOM element for output
   */
  attach( element: HTMLElement ): void {
    this.element = element;
    this.createCursor();
  }

  /**
   * Create blinking cursor element
   */
  private createCursor(): void {
    if ( ! this.element ) {
      return;
    }

    this.cursorElement = document.createElement( 'span' );
    this.cursorElement.className = 'cursor';
    this.cursorElement.textContent = '';
  }

  /**
   * Sleep utility with abort support
   */
  private sleep( ms: number, signal?: AbortSignal ): Promise<void> {
    return new Promise( ( resolve, reject ) => {
      const timeout = setTimeout( resolve, ms );
      signal?.addEventListener( 'abort', () => {
        clearTimeout( timeout );
        reject( new DOMException( 'Aborted', 'AbortError' ) );
      } );
    } );
  }

  /**
   * Stream a single character
   */
  private async streamChar(
    char: string,
    target: HTMLElement,
    signal: AbortSignal
  ): Promise<void> {
    target.textContent += char;
    this.options.onChar?.( char );
    await this.sleep( this.options.speed!, signal );
  }

  /**
   * Stream a single line of text
   */
  async streamText(
    text: string,
    type: TerminalLine['type'] = 'output'
  ): Promise<void> {
    if ( ! this.element ) {
      throw new Error( 'ConsoleStream not attached to element' );
    }

    this.isStreaming = true;
    this.abortController = new AbortController();
    const signal = this.abortController.signal;

    // Create line container
    const lineEl = document.createElement( 'div' );
    lineEl.className = `terminal-line terminal-${type}`;
    this.element.appendChild( lineEl );

    // Add cursor to line
    if ( this.cursorElement ) {
      lineEl.appendChild( this.cursorElement );
    }

    try {
      // Stream each character
      for ( const char of text ) {
        if ( this.cursorElement ) {
          lineEl.insertBefore(
            document.createTextNode( char ),
            this.cursorElement
          );
        }
        else {
          lineEl.textContent += char;
        }
        this.options.onChar?.( char );
        await this.sleep( this.options.speed!, signal );
      }

      this.options.onLine?.( text );
    }
    catch ( e ) {
      if ( e instanceof DOMException && e.name === 'AbortError' ) {
        // Streaming was aborted
        return;
      }
      throw e;
    }
    finally {
      this.isStreaming = false;
    }
  }

  /**
   * Stream multiple lines with delays between them
   */
  async streamLines( lines: TerminalLine[] ): Promise<void> {
    if ( ! this.element ) {
      throw new Error( 'ConsoleStream not attached to element' );
    }

    this.abortController = new AbortController();
    const signal = this.abortController.signal;

    try {
      for ( const line of lines ) {
        await this.streamText( line.text, line.type );
        await this.sleep( this.options.lineDelay!, signal );
      }
      this.options.onComplete?.();
    }
    catch ( e ) {
      if ( e instanceof DOMException && e.name === 'AbortError' ) {
        return;
      }
      throw e;
    }
  }

  /**
   * Instantly print a line (no streaming)
   */
  printLine( text: string, type: TerminalLine['type'] = 'output' ): void {
    if ( ! this.element ) {
      return;
    }

    const lineEl = document.createElement( 'div' );
    lineEl.className = `terminal-line terminal-${type}`;
    lineEl.textContent = text;
    this.element.appendChild( lineEl );

    // Move cursor to new line
    if ( this.cursorElement ) {
      const cursorLine = document.createElement( 'div' );
      cursorLine.className = 'terminal-line';
      cursorLine.appendChild( this.cursorElement );
      this.element.appendChild( cursorLine );
    }
  }

  /**
   * Print a preformatted block (for ASCII art)
   */
  printPre( text: string, type: TerminalLine['type'] = 'output' ): void {
    if ( ! this.element ) {
      return;
    }

    const preEl = document.createElement( 'pre' );
    preEl.className = `terminal-line terminal-pre terminal-${type}`;
    preEl.textContent = text;
    this.element.appendChild( preEl );

    // Move cursor after pre block
    if ( this.cursorElement ) {
      const cursorLine = document.createElement( 'div' );
      cursorLine.className = 'terminal-line';
      cursorLine.appendChild( this.cursorElement );
      this.element.appendChild( cursorLine );
    }
  }

  /**
   * Print prompt prefix (like "> " or "$ ")
   */
  async streamPrompt( prompt: string = '> ' ): Promise<void> {
    await this.streamText( prompt, 'prompt' );
  }

  /**
   * Clear terminal contents
   */
  clear(): void {
    if ( ! this.element ) {
      return;
    }

    // Fade out effect
    this.element.style.opacity = '0';
    setTimeout( () => {
      this.element!.innerHTML = '';
      this.element!.style.opacity = '1';

      // Re-add cursor
      if ( this.cursorElement ) {
        const cursorLine = document.createElement( 'div' );
        cursorLine.className = 'terminal-line';
        cursorLine.appendChild( this.cursorElement );
        this.element!.appendChild( cursorLine );
      }
    }, 200 );
  }

  /**
   * Stop any current streaming
   */
  abort(): void {
    this.abortController?.abort();
    this.isStreaming = false;
  }

  /**
   * Check if currently streaming
   */
  get streaming(): boolean {
    return this.isStreaming;
  }

  /**
   * Scroll to bottom of terminal
   */
  scrollToBottom(): void {
    if ( this.element ) {
      this.element.scrollTop = this.element.scrollHeight;
    }
  }
}

/**
 * Factory function for quick setup
 */
export function createConsoleStream(
  element: HTMLElement,
  options?: Partial<StreamOptions>
): ConsoleStream {
  const stream = new ConsoleStream( options );
  stream.attach( element );

  return stream;
}
