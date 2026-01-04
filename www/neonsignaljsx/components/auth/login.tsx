import { webAuthnLogin } from '../../scripts/web_auth/web_auth';
import { markEnrolled } from '../../scripts/auth_state';

type LoginProps = {
  onSuccess: () => void;
};

export const Login = ( { onSuccess }: LoginProps ) => {
  const handleLogin = async () => {
    const ok = await webAuthnLogin();
    if ( ok ) {
      markEnrolled();
      onSuccess();
    }
  };

  return (
    <button
      type="button"
      className="pill"
      title="Authenticate with Security Key"
      style="color:#FFEE00;background:transparent;border:none;cursor:pointer;filter:drop-shadow(0 0 4px rgba(255,238,0,0.5))"
      onClick={handleLogin}
    >
      ⚷
    </button>
  );
};
