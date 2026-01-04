type StreamFactory = () => EventSource;

type StreamEntry = {
  name: string;
  factory: StreamFactory;
  throttleMs: number;
  source?: EventSource;
  nextAllowedAt: number;
};

const streams = new Map<string, StreamEntry>();
type GearMode = 'turbo' | 'throttle' | 'kill';
let gearMode: GearMode = 'turbo';

function startStream( entry: StreamEntry ) {
  if ( entry.source ) {
    return;
  }
  entry.source = entry.factory();
}

function stopStream( entry: StreamEntry ) {
  if ( entry.source ) {
    entry.source.close();
    entry.source = undefined;
  }
}

export function registerStream( name: string, factory: StreamFactory ) {
  registerStreamWithThrottle( name, factory, 2000 );
}

export function registerStreamWithThrottle(
  name: string,
  factory: StreamFactory,
  throttleMs = 2000
) {
  if ( streams.has( name ) ) {
    return;
  }
  const entry: StreamEntry = {
    name,
    factory,
    throttleMs,
    nextAllowedAt: 0,
  };
  streams.set( name, entry );
  if ( gearMode !== 'kill' ) {
    startStream( entry );
  }
}

export function setGearMode( mode: GearMode ) {
  gearMode = mode;
  streams.forEach( ( entry ) => {
    if ( gearMode === 'kill' ) {
      stopStream( entry );
    }
    else {
      startStream( entry );
    }
  } );
}

export function shouldProcess( name: string ) {
  if ( gearMode === 'kill' ) {
    return false;
  }
  if ( gearMode === 'turbo' ) {
    return true;
  }
  const entry = streams.get( name );
  if ( ! entry ) {
    return false;
  }
  const now = Date.now();
  if ( now >= entry.nextAllowedAt ) {
    entry.nextAllowedAt = now + entry.throttleMs;

    return true;
  }

  return false;
}

export function getGearMode() {
  return gearMode;
}
