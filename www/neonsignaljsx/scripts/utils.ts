export const b64urlToBuf = ( b64: string ) =>
  Uint8Array.from(
    atob( b64.replace( /-/g, '+' ).replace( /_/g, '/' ) ),
    c => c.charCodeAt( 0 )
  );

export const bufToB64url = ( buf: ArrayBuffer ) =>
  btoa( String.fromCharCode( ...new Uint8Array( buf ) ) )
    .replace( /\+/g, '-' )
    .replace( /\//g, '_' )
    .replace( /=+$/g, '' );

