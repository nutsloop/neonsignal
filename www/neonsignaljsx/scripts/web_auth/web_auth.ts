import { bufToB64url, b64urlToBuf } from '../utils';
export async function webAuthnLogin(): Promise<boolean> {
  const statusEl = document.getElementById( 'auth-status' );
  const setStatus = ( msg: string ) => {
    if ( statusEl ) {
      statusEl.textContent = msg;
    }
  };
  const setResult = ( msg: string ) => {
    try {
      localStorage.setItem( 'ns_auth_result', msg );
    }
    catch ( e ) {
      console.error( e );
    }
  };
  try {
    setStatus( 'Requesting challenge…' );
    const optsRes = await fetch( '/api/auth/login/options', { method: 'POST' } );
    if ( ! optsRes.ok ) {
      setStatus( `Options error: ${optsRes.status}` );
      setResult( `fail: options ${optsRes.status}` );

      return false;
    }
    const optsJson = await optsRes.json();
    const challenge = b64urlToBuf( optsJson.challenge );
    const allowCredentials = ( optsJson.allowCredentials || [] ).map( ( c: any ) => ( {
      type: c.type,
      id: b64urlToBuf( c.id )
    } ) );

    setStatus( 'Awaiting security key…' );
    const cred: any = await navigator.credentials.get( {
      publicKey: {
        challenge,
        allowCredentials,
        rpId: optsJson.rpId,
        userVerification: 'preferred',
        timeout: optsJson.timeout || 60000
      }
    } );

    const payload = {
      credentialId: bufToB64url( cred.rawId ),
      clientDataJSON: bufToB64url( cred.response.clientDataJSON ),
      authenticatorData: bufToB64url( cred.response.authenticatorData ),
      signature: bufToB64url( cred.response.signature )
    };

    setStatus( 'Verifying…' );
    const finishRes = await fetch( '/api/auth/login/finish', {
      method: 'POST',
      headers: { 'content-type': 'application/json' },
      body: JSON.stringify( payload )
    } );
    const finishText = await finishRes.text();
    if ( ! finishRes.ok ) {
      setStatus( `Auth failed: ${finishRes.status} ${finishText}` );
      setResult( `fail: finish ${finishRes.status} ${finishText}` );

      return false;
    }
    setStatus( 'Authenticated' );
    setResult( 'ok' );

    return true;
  }
  catch ( err: any ) {
    setStatus( `Auth error: ${String( err )}` );
    setResult( `fail: ${String( err )}` );

    return false;
  }
}
