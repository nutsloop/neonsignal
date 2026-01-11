/**
 * Glitch Effects - Visual distortion and interference effects
 * Separation of Concerns: This module handles ONLY glitch/distortion effects
 */

export interface GlitchOptions {
  intensity?: number; // 0-1, how intense the glitch
  interval?: number; // ms between auto-glitches
  duration?: number; // ms for each glitch
  autoStart?: boolean; // start glitching automatically
  glitchChars?: string; // characters to use for text scramble
}

const DEFAULT_OPTIONS: GlitchOptions = {
  intensity: 0.5,
  interval: 8000,
  duration: 200,
  autoStart: true,
  glitchChars: '!@#$%^&*()_+-=[]{}|;:,.<>?/~`░▒▓█▄▀■□▪▫',
};

export class GlitchEffect {
  private options: GlitchOptions;
  private intervalId: number | null = null;
  private elements: Map<HTMLElement, string> = new Map();

  constructor( options: Partial<GlitchOptions> = {} ) {
    this.options = { ...DEFAULT_OPTIONS, ...options };
  }

  /**
   * Register an element for glitch effects
   * Stores original text for restoration
   */
  register( element: HTMLElement ): void {
    const text = element.getAttribute( 'data-text' ) || element.textContent || '';
    element.setAttribute( 'data-text', text );
    this.elements.set( element, text );
  }

  /**
   * Trigger glitch on a specific element
   */
  glitchElement( element: HTMLElement ): void {
    element.classList.add( 'active' );

    setTimeout( () => {
      element.classList.remove( 'active' );
    }, this.options.duration );
  }

  /**
   * Trigger text scramble effect
   */
  scrambleText( element: HTMLElement, duration?: number ): void {
    const originalText = this.elements.get( element ) || element.textContent || '';
    const chars = this.options.glitchChars!;
    const scrambleDuration = duration || this.options.duration!;
    const steps = 10;
    const stepDuration = scrambleDuration / steps;
    let step = 0;

    const scramble = () => {
      if ( step >= steps ) {
        element.textContent = originalText;

        return;
      }

      // Progressively reveal more of the original text
      const revealRatio = step / steps;
      let result = '';

      for ( let i = 0; i < originalText.length; i ++ ) {
        if ( Math.random() < revealRatio ) {
          result += originalText[ i ];
        }
        else {
          result += chars[ Math.floor( Math.random() * chars.length ) ];
        }
      }

      element.textContent = result;
      step ++;
      setTimeout( scramble, stepDuration );
    };

    scramble();
  }

  /**
   * Trigger screen shake on container
   */
  shake( element: HTMLElement ): void {
    element.classList.add( 'shake' );

    setTimeout( () => {
      element.classList.remove( 'shake' );
    }, 300 );
  }

  /**
   * RGB split effect (via CSS class)
   */
  rgbSplit( element: HTMLElement ): void {
    element.style.setProperty( '--glitch-offset', `${Math.random() * 4 - 2}px` );
    this.glitchElement( element );
  }

  /**
   * Trigger random glitch on all registered elements
   */
  triggerRandom(): void {
    const elements = Array.from( this.elements.keys() );
    if ( elements.length === 0 ) {
      return;
    }

    // Pick random element
    const element = elements[ Math.floor( Math.random() * elements.length ) ];

    // Random effect
    const effects = [ 'glitch', 'scramble', 'rgbSplit' ];
    const effect = effects[ Math.floor( Math.random() * effects.length ) ];

    switch ( effect ) {
      case 'glitch':
        this.glitchElement( element );
        break;
      case 'scramble':
        this.scrambleText( element );
        break;
      case 'rgbSplit':
        this.rgbSplit( element );
        break;
      // skip default
    }
  }

  /**
   * Start automatic glitch interval
   */
  start(): void {
    if ( this.intervalId ) {
      return;
    }

    this.intervalId = window.setInterval( () => {
      // Random chance based on intensity
      if ( Math.random() < this.options.intensity! ) {
        this.triggerRandom();
      }
    }, this.options.interval );
  }

  /**
   * Stop automatic glitching
   */
  stop(): void {
    if ( this.intervalId ) {
      clearInterval( this.intervalId );
      this.intervalId = null;
    }
  }

  /**
   * Clean up
   */
  destroy(): void {
    this.stop();
    this.elements.clear();
  }
}

/**
 * Static noise generator for background effects
 */
export class StaticNoise {
  private canvas: HTMLCanvasElement | null = null;
  private ctx: CanvasRenderingContext2D | null = null;
  private animationId: number | null = null;
  private opacity: number;

  constructor( opacity: number = 0.03 ) {
    this.opacity = opacity;
  }

  /**
   * Create and attach noise canvas overlay
   */
  attach( container: HTMLElement ): void {
    this.canvas = document.createElement( 'canvas' );
    this.canvas.style.cssText = `
      position: fixed;
      inset: 0;
      pointer-events: none;
      z-index: 999;
      opacity: ${this.opacity};
      mix-blend-mode: overlay;
    `;

    this.ctx = this.canvas.getContext( '2d' );
    container.appendChild( this.canvas );

    this.resize();
    window.addEventListener( 'resize', this.resize.bind( this ) );
  }

  /**
   * Handle resize
   */
  private resize(): void {
    if ( ! this.canvas ) {
      return;
    }
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;
  }

  /**
   * Generate noise frame
   */
  private generateNoise(): void {
    if ( ! this.ctx || ! this.canvas ) {
      return;
    }

    const imageData = this.ctx.createImageData(
      this.canvas.width,
      this.canvas.height
    );
    const data = imageData.data;

    for ( let i = 0; i < data.length; i += 4 ) {
      const value = Math.random() * 255;
      data[ i ] = value; // R
      data[ i + 1 ] = value; // G
      data[ i + 2 ] = value; // B
      data[ i + 3 ] = 255; // A
    }

    this.ctx.putImageData( imageData, 0, 0 );
  }

  /**
   * Start noise animation
   */
  start(): void {
    const animate = () => {
      this.generateNoise();
      this.animationId = requestAnimationFrame( animate );
    };
    animate();
  }

  /**
   * Stop noise animation
   */
  stop(): void {
    if ( this.animationId ) {
      cancelAnimationFrame( this.animationId );
      this.animationId = null;
    }
  }

  /**
   * Clean up
   */
  destroy(): void {
    this.stop();
    this.canvas?.remove();
    window.removeEventListener( 'resize', this.resize.bind( this ) );
  }
}

/**
 * Factory functions
 */
export function createGlitchEffect( options?: Partial<GlitchOptions> ): GlitchEffect {
  const effect = new GlitchEffect( options );
  if ( options?.autoStart !== false ) {
    effect.start();
  }

  return effect;
}

export function createStaticNoise(
  container: HTMLElement,
  opacity?: number
): StaticNoise {
  const noise = new StaticNoise( opacity );
  noise.attach( container );
  noise.start();

  return noise;
}
