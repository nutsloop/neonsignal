export const UserCheck = () => {
  const handleCheck = async () => {
    const input = document.getElementById( 'user-check-name' ) as HTMLInputElement | null;
    const statusEl = document.getElementById( 'user-check-status' );
    const user = input?.value.trim() ?? '';

    if ( ! user ) {
      if ( statusEl ) {
        statusEl.textContent = 'Provide an email to check.';
      }

      return;
    }

    if ( statusEl ) {
      statusEl.textContent = 'Checking database...';
    }

    try {
      const res = await fetch( '/api/auth/user/check', {
        method: 'POST',
        headers: {
          'x-user': user,
        },
      } );
      if ( ! res.ok ) {
        const msg = await res.text();
        if ( statusEl ) {
          statusEl.textContent = `Check failed (${res.status}): ${msg}`;
        }

        return;
      }
      const data = await res.json();
      if ( statusEl ) {
        statusEl.textContent = data.exists
          ? `Email '${data.user}' exists in DB.`
          : `Email '${data.user}' not found.`;
      }
    }
    catch ( err ) {
      if ( statusEl ) {
        statusEl.textContent = 'Network error while checking user.';
      }
      console.error( err );
    }
  };

  return (
    <section className="panel auth">
      <h2>Check User in DB</h2>
      <p className="lede">Verify DB integration by checking for an existing user.</p>
      <label>
        Email
        <input type="email" id="user-check-name" placeholder="you@example.com" />
      </label>
      <button type="button" className="pill" onClick={handleCheck}>
        Check User
      </button>
      <p id="user-check-status" className="status" aria-live="polite"></p>
    </section>
  );
};
