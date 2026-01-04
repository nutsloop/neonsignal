/**
 * Enrollment state persistence using localStorage.
 * Tracks whether a user has enrolled a WebAuthn credential.
 */

const ENROLLMENT_KEY = 'ns_enrolled';

/**
 * Mark that the user has enrolled a security key.
 * Called after successful WebAuthn enrollment.
 */
export function markEnrolled(): void {
  try {
    localStorage.setItem( ENROLLMENT_KEY, 'true' );
  }
  catch ( e ) {
    console.error( 'Failed to save enrollment state:', e );
  }
}

/**
 * Check if the user has previously enrolled a security key.
 * Returns true if enrollment flag is set in localStorage.
 */
export function isEnrolled(): boolean {
  try {
    return localStorage.getItem( ENROLLMENT_KEY ) === 'true';
  }
  catch ( e ) {
    console.error( 'Failed to read enrollment state:', e );

    return false;
  }
}

/**
 * Clear the enrollment state.
 * Useful for testing or if user wants to re-register.
 */
export function clearEnrollment(): void {
  try {
    localStorage.removeItem( ENROLLMENT_KEY );
  }
  catch ( e ) {
    console.error( 'Failed to clear enrollment state:', e );
  }
}
