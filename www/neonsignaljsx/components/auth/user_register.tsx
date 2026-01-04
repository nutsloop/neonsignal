import { registerUser, RegisterResponse } from '../../scripts/web_auth/user_enrollment';

type RegisterState = 'form' | 'pending' | 'error';

interface State {
  state: RegisterState;
  email: string;
  displayName: string;
  token: string;
  error: string;
}

const state: State = {
  state: 'form',
  email: '',
  displayName: '',
  token: '',
  error: ''
};

function render() {
  const container = document.getElementById( 'user-register-container' );
  if ( ! container ) {
    return;
  }

  container.innerHTML = '';

  if ( state.state === 'form' ) {
    container.appendChild( renderForm() );
  }
  else if ( state.state === 'pending' ) {
    container.appendChild( renderPending() );
  }
  else {
    container.appendChild( renderError() );
  }
}

function renderForm(): HTMLElement {
  const section = document.createElement( 'div' );
  section.innerHTML = `
    <p class="lede">
      Create a new account. After registration, verify your email using the provided curl command.
    </p>
    <form id="user-register-form" novalidate>
      <label>
        Email
        <input type="email" id="user-email" placeholder="you@example.com" required pattern="[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}" />
      </label>
      <label>
        Display Name
        <input type="text" id="user-display-name" placeholder="Your Name" required />
      </label>
      <button type="submit" class="pill" id="user-register-btn">
        Register
      </button>
      <p id="user-register-status" class="status" aria-live="polite"></p>
    </form>
  `;

  const form = section.querySelector( '#user-register-form' ) as HTMLFormElement;
  form.addEventListener( 'submit', ( e ) => {
    e.preventDefault();
    const emailInput = form.querySelector( '#user-email' ) as HTMLInputElement;
    const displayNameInput = form.querySelector( '#user-display-name' ) as HTMLInputElement;
    const statusEl = form.querySelector( '#user-register-status' ) as HTMLElement;

    // Check HTML5 validity
    if ( ! emailInput.checkValidity() ) {
      statusEl.textContent = 'Please enter a valid email address.';

      return;
    }
    if ( ! displayNameInput.checkValidity() ) {
      statusEl.textContent = 'Display name is required.';

      return;
    }

    handleRegister();
  } );

  return section;
}

function renderPending(): HTMLElement {
  const section = document.createElement( 'div' );
  const curlCmd = `curl -X POST ${window.location.origin}/api/auth/user/verify \\
  -H "Content-Type: application/json" \\
  -d '{"email":"${state.email}","token":"${state.token}"}'`;

  section.innerHTML = `
    <p class="lede success">
      ✓ Registration successful! Verify your account using the command below.
    </p>
    <div class="verification-box">
      <p><strong>Email:</strong> ${state.email}</p>
      <p><strong>Verification Token:</strong></p>
      <code class="token">${state.token}</code>
      <p><strong>Verify with curl:</strong></p>
      <pre class="curl-command">${curlCmd}</pre>
      <button type="button" class="pill small" id="copy-curl-btn">
        Copy Command
      </button>
    </div>
    <p class="lede">
      After verifying, return here to enroll your security key.
    </p>
    <button type="button" class="pill secondary" id="back-to-form-btn">
      Register Another
    </button>
  `;

  const copyBtn = section.querySelector( '#copy-curl-btn' ) as HTMLButtonElement;
  copyBtn.addEventListener( 'click', () => {
    navigator.clipboard.writeText( curlCmd ).then( () => {
      copyBtn.textContent = 'Copied!';
      setTimeout( () => {
        copyBtn.textContent = 'Copy Command';
      }, 2000 );
    } );
  } );

  const backBtn = section.querySelector( '#back-to-form-btn' ) as HTMLButtonElement;
  backBtn.addEventListener( 'click', () => {
    state.state = 'form';
    state.token = '';
    state.email = '';
    state.displayName = '';
    render();
  } );

  return section;
}

function renderError(): HTMLElement {
  const section = document.createElement( 'div' );
  section.innerHTML = `
    <p class="lede error">
      ✗ Registration failed: ${state.error}
    </p>
    <button type="button" class="pill secondary" id="retry-btn">
      Try Again
    </button>
  `;

  const retryBtn = section.querySelector( '#retry-btn' ) as HTMLButtonElement;
  retryBtn.addEventListener( 'click', () => {
    state.state = 'form';
    state.error = '';
    render();
  } );

  return section;
}

async function handleRegister() {
  const emailInput = document.getElementById( 'user-email' ) as HTMLInputElement;
  const displayNameInput = document.getElementById( 'user-display-name' ) as HTMLInputElement;
  const statusEl = document.getElementById( 'user-register-status' );
  const btn = document.getElementById( 'user-register-btn' ) as HTMLButtonElement;

  const email = emailInput?.value.trim();
  const displayName = displayNameInput?.value.trim();

  if ( ! email || ! displayName ) {
    if ( statusEl ) {
      statusEl.textContent = 'Email and display name are required';
    }

    return;
  }

  btn.disabled = true;
  if ( statusEl ) {
    statusEl.textContent = 'Registering...';
  }

  const result: RegisterResponse = await registerUser( email, displayName );

  if ( result.ok && result.token ) {
    state.email = email;
    state.displayName = displayName;
    state.token = result.token;
    state.state = 'pending';
    render();
  }
  else {
    state.error = result.error || 'Unknown error';
    state.state = 'error';
    render();
  }
}

export const UserRegister = () => {
  // Initialize render after mount
  setTimeout( render, 0 );

  return (
    <section className="panel auth" id="user-register-section">
      <h2>Create Account</h2>
      <div id="user-register-container"></div>
    </section>
  );
};

export function resetUserRegister() {
  state.state = 'form';
  state.email = '';
  state.displayName = '';
  state.token = '';
  state.error = '';
}
