# Plan: Integrate `nutsloop::args` with `spin` command and `--systemd`

Goal: add strict CLI parsing for both executables (`neonsignal`, `neonsignal_redirect`) using `nutsloop::args` with a single required command `spin`, a `--systemd` flag that bypasses the `spin` requirement when run as a service, and flag forms that must be `--key=value` (no space-separated values). CLI flags mirror existing environment variables but must not override them when the env is set. Include a CheckerClass (strict validation) and HelperClass (help text). Vendor the args library as `subprojects/args.wrap`. Avoid behavior changes beyond CLI parsing and help output.

## Phase 1 — Define CLI contract and mapping
- Enumerate env vars
  - `neonsignal`: `NEONSIGNAL_THREADS`, `NEONSIGNAL_HOST`, `NEONSIGNAL_PORT`, `NEONSIGNAL_WEBAUTHN_DOMAIN`, `NEONSIGNAL_WEBAUTHN_ORIGIN`, `NEONSIGNAL_DB_PATH`
  - `neonsignal_redirect`: `REDIRECT_INSTANCES`, `REDIRECT_PORT`, `REDIRECT_TARGET_PORT`, `REDIRECT_HOST`, `ACME_WEBROOT`
- Define CLI options (`--key=value` only)
  - `neonsignal`: `--threads=<n>`, `--host=<addr>`, `--port=<n>`, `--webauthn-domain=<id>`, `--webauthn-origin=<url>`, `--db-path=<path>`
  - `neonsignal_redirect`: `--instances=<n>`, `--port=<n>`, `--target-port=<n>`, `--host=<addr>`, `--acme-webroot=<path>`
  - Shared: `--systemd` (bool), `spin` command required unless `--systemd` present
- Conflict rule
  - Environment remains authoritative; CLI cannot override env values
  - If env exists, still validate CLI but do not apply it
  - If env missing, CLI values may be applied as defaults

## Phase 2 — Vendor `nutsloop::args` and integrate in Meson
- Add `subprojects/args.wrap` pointing to `https://github.com/nutsloop/args`
- In Meson, add `dependency('args', fallback: ['args', 'args_dep'])` and link only into `neonsignal` and `neonsignal_redirect`
- Ensure Meson version and C++23 settings are compatible

## Phase 3 — Implement strict parser scaffolding
- CheckerClass
  - Enforce: single command must be `spin` unless `--systemd` set
  - Validate numeric bounds (ports 1–65535, instances > 0)
  - Validate non-empty strings (host, webauthn-domain, webauthn-origin, db-path, acme-webroot)
  - Reject `--key value` form; accept `--key=value` only
  - Provide explicit error messages for invalid command/flag/value forms
- HelperClass
  - Provide usage examples for both executables
  - List flags with env equivalents
  - Mention `spin` requirement and `--systemd` bypass
  - State `--key=value` requirement and env precedence
- Placement
  - Shared helper header/impl under `include/neonsignal/` and `src/neonsignal/`, following project naming rules

## Phase 4 — Apply parser to `neonsignal`
- Update `src/main.c++` to parse args before constructing `Server`
- Enforce `spin` or `--systemd`
- Apply CLI values only if env var not set; preserve existing defaults otherwise
- Keep current env-based behavior unchanged when CLI absent

## Phase 5 — Apply parser to `neonsignal_redirect`
- Update `src/redirect_main.c++` similarly
- Enforce `spin` or `--systemd`; apply CLI only when env missing
- Preserve existing defaults when neither env nor CLI is provided

## Phase 6 — Validation (deferred)
- Do not run builds automatically
- Optional manual checks when requested:
  - `./neonsignal spin --host=0.0.0.0 --port=9443`
  - `./neonsignal --systemd --host=0.0.0.0`
  - `./neonsignal_redirect spin --port=9090 --target-port=443`
  - Verify env vars override CLI values when set
