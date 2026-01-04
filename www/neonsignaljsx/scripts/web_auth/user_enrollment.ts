import { bufToB64url, b64urlToBuf } from '../utils';

export interface RegisterResponse {
  ok: boolean;
  token?: string;
  error?: string;
}

export interface EnrollStartResponse {
  ok: boolean;
  sessionId?: string;
  error?: string;
}

export interface EnrollFinishResponse {
  ok: boolean;
  error?: string;
}

/**
 * Register a new user account.
 * Returns a verification token that must be used via curl to verify the account.
 */
export async function registerUser(
  email: string,
  displayName: string
): Promise<RegisterResponse> {
  try {
    const res = await fetch( '/api/auth/user/register', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify( { email, display_name: displayName } )
    } );

    const text = await res.text();
    let json: Record<string, unknown>;

    try {
      json = JSON.parse( text );
    }
    catch {
      return { ok: false, error: `Invalid response (${res.status}): ${text.slice( 0, 100 )}` };
    }

    if ( ! res.ok ) {
      return { ok: false, error: ( json.error as string ) || `Registration failed: ${res.status}` };
    }

    return { ok: true, token: json.token as string };
  }
  catch ( err: unknown ) {
    return { ok: false, error: `Network error: ${String( err )}` };
  }
}

/**
 * Start enrollment session for a verified user.
 * This requires the user to have already verified their account via curl.
 * On success, sets a pre_webauthn session cookie.
 */
export async function startEnrollment( email: string ): Promise<EnrollStartResponse> {
  try {
    const res = await fetch( '/api/auth/user/verify', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify( { email, token: '' } ) // Empty token to check if verified
    } );

    // If token is required, user hasn't been verified yet
    if ( ! res.ok ) {
      const json = await res.json();

      return { ok: false, error: json.error || 'Account not verified' };
    }

    return { ok: true };
  }
  catch ( err: unknown ) {
    return { ok: false, error: String( err ) };
  }
}

/**
 * Get WebAuthn enrollment options.
 * Requires pre_webauthn session cookie (set by /api/auth/user/verify).
 */
export async function getEnrollmentOptions(): Promise<{
  ok: boolean;
  options?: any;
  error?: string;
}> {
  try {
    const res = await fetch( '/api/auth/user/enroll', {
      method: 'GET',
      credentials: 'include'
    } );

    if ( ! res.ok ) {
      const json = await res.json();

      return { ok: false, error: json.error || `Failed to get options: ${res.status}` };
    }

    const options = await res.json();

    return { ok: true, options };
  }
  catch ( err: unknown ) {
    return { ok: false, error: String( err ) };
  }
}

/**
 * Complete WebAuthn enrollment.
 * Requires pre_webauthn session cookie and WebAuthn attestation.
 */
export async function finishEnrollment( attestation: {
  credentialId: string;
  clientDataJSON: string;
  attestationObject: string;
} ): Promise<EnrollFinishResponse> {
  try {
    const res = await fetch( '/api/auth/user/enroll', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      credentials: 'include',
      body: JSON.stringify( attestation )
    } );

    if ( ! res.ok ) {
      const json = await res.json();

      return { ok: false, error: json.error || `Enrollment failed: ${res.status}` };
    }

    return { ok: true };
  }
  catch ( err: unknown ) {
    return { ok: false, error: String( err ) };
  }
}

/**
 * Full WebAuthn enrollment flow.
 * 1. Get enrollment options from server
 * 2. Create credential via WebAuthn API
 * 3. Send attestation to server
 */
export async function enrollWebAuthn(): Promise<EnrollFinishResponse> {
  // Step 1: Get options
  const optionsResult = await getEnrollmentOptions();
  if ( ! optionsResult.ok || ! optionsResult.options ) {
    return { ok: false, error: optionsResult.error || 'Failed to get enrollment options' };
  }

  const opts = optionsResult.options;

  try {
    // Step 2: Create credential
    const challenge = b64urlToBuf( opts.challenge );
    const userId = b64urlToBuf( opts.user.id );

    const cred: PublicKeyCredential = await navigator.credentials.create( {
      publicKey: {
        rp: opts.rp,
        user: {
          id: userId,
          name: opts.user.name,
          displayName: opts.user.displayName
        },
        challenge,
        pubKeyCredParams: opts.pubKeyCredParams,
        authenticatorSelection: opts.authenticatorSelection,
        timeout: opts.timeout || 60000
      }
    } ) as PublicKeyCredential;

    if ( ! cred ) {
      return { ok: false, error: 'No credential returned' };
    }

    const response = cred.response as AuthenticatorAttestationResponse;

    // Step 3: Send attestation
    const attestation = {
      credentialId: bufToB64url( cred.rawId ),
      clientDataJSON: bufToB64url( response.clientDataJSON ),
      attestationObject: bufToB64url( response.attestationObject )
    };

    return await finishEnrollment( attestation );
  }
  catch ( err: unknown ) {
    return { ok: false, error: String( err ) };
  }
}
