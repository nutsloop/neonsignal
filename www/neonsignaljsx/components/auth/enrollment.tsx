import { markEnrolled } from '../../scripts/auth_state';
import { startEnrollment, enrollWebAuthn, EnrollFinishResponse } from '../../scripts/web_auth/user_enrollment';

type EnrollmentState = 'form' | 'enrolling' | 'success' | 'error';

interface State {
  state: EnrollmentState;
  email: string;
  error: string;
}

const state: State = {
  state: 'form',
  email: '',
  error: ''
};

let onSuccessCallback: ( () => void ) | null = null;

function render() {
  const container = document.getElementById( 'enrollment-container' );
  if ( ! container ) {
    return;
  }

  container.innerHTML = '';

  if ( state.state === 'form' ) {
    container.appendChild( renderForm() );
  }
  else if ( state.state === 'enrolling' ) {
    container.appendChild( renderEnrolling() );
  }
  else if ( state.state === 'success' ) {
    container.appendChild( renderSuccess() );
  }
  else {
    container.appendChild( renderError() );
  }
}

function renderForm(): HTMLElement {
  const section = document.createElement( 'div' );
  section.innerHTML = `
    <p class="lede">
      Already verified your account? Enter your email to enroll your security key.
    </p>
    <form id="enroll-form" novalidate>
      <label>
        Email
        <input type="email" id="enroll-email" placeholder="you@example.com" required pattern="[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}" />
      </label>
      <button type="submit" class="pill" id="enroll-start-btn">
        Start Enrollment
      </button>
      <p id="enroll-status" class="status" aria-live="polite"></p>
    </form>
  `;

  const form = section.querySelector( '#enroll-form' ) as HTMLFormElement;
  form.addEventListener( 'submit', ( e ) => {
    e.preventDefault();
    const emailInput = form.querySelector( '#enroll-email' ) as HTMLInputElement;
    const statusEl = form.querySelector( '#enroll-status' ) as HTMLElement;

    if ( ! emailInput.checkValidity() ) {
      statusEl.textContent = 'Please enter a valid email address.';

      return;
    }

    handleStartEnrollment();
  } );

  return section;
}

function renderEnrolling(): HTMLElement {
  const section = document.createElement( 'div' );
  section.innerHTML = `
    <p class="lede">
      Touch your security key or use biometric authentication...
    </p>
    <div class="spinner"></div>
    <p id="enroll-status" class="status" aria-live="polite">Waiting for authenticator...</p>
  `;

  return section;
}

function renderSuccess(): HTMLElement {
  // Mark enrollment in localStorage for returning users
  markEnrolled();

  const section = document.createElement( 'div' );
  section.innerHTML = `
    <p class="lede success">
      ✓ Security key enrolled successfully!
    </p>
    <p>You can now use your security key to log in.</p>
    <button type="button" class="pill" id="login-now-btn">
      Continue to Login
    </button>
  `;

  const btn = section.querySelector( '#login-now-btn' ) as HTMLButtonElement;
  btn.addEventListener( 'click', () => {
    if ( onSuccessCallback ) {
      onSuccessCallback();
    }
    else {
      // Reload to show login
      window.location.reload();
    }
  } );

  return section;
}

function renderError(): HTMLElement {
  const section = document.createElement( 'div' );
  section.innerHTML = `
    <p class="lede error">
      ✗ Enrollment failed: ${state.error}
    </p>
    <button type="button" class="pill secondary" id="retry-enroll-btn">
      Try Again
    </button>
  `;

  const retryBtn = section.querySelector( '#retry-enroll-btn' ) as HTMLButtonElement;
  retryBtn.addEventListener( 'click', () => {
    state.state = 'form';
    state.error = '';
    render();
  } );

  return section;
}

async function handleStartEnrollment() {
  const emailInput = document.getElementById( 'enroll-email' ) as HTMLInputElement;
  const statusEl = document.getElementById( 'enroll-status' );
  const btn = document.getElementById( 'enroll-start-btn' ) as HTMLButtonElement;

  const email = emailInput?.value.trim();

  if ( ! email ) {
    if ( statusEl ) {
      statusEl.textContent = 'Email is required';
    }

    return;
  }

  btn.disabled = true;
  if ( statusEl ) {
    statusEl.textContent = 'Checking verification status...';
  }

  state.email = email;

  // Check if user is verified and start enrollment session
  const startResult = await startEnrollment( email );

  if ( ! startResult.ok ) {
    state.error = startResult.error || 'Account not verified or not found';
    state.state = 'error';
    render();

    return;
  }

  // User is verified, proceed with WebAuthn enrollment
  state.state = 'enrolling';
  render();

  const enrollResult: EnrollFinishResponse = await enrollWebAuthn();

  if ( enrollResult.ok ) {
    state.state = 'success';
    render();
  }
  else {
    state.error = enrollResult.error || 'WebAuthn enrollment failed';
    state.state = 'error';
    render();
  }
}

export interface EnrollmentProps {
  onSuccess?: () => void;
}

export const Enrollment = ( { onSuccess }: EnrollmentProps ) => {
  onSuccessCallback = onSuccess || null;

  // Initialize render after mount
  setTimeout( render, 0 );

  return (
    <section className="panel auth">
      <h2>Enroll Security Key</h2>
      <div id="enrollment-container"></div>
    </section>
  );
};

export function resetEnrollment() {
  state.state = 'form';
  state.email = '';
  state.error = '';
}
