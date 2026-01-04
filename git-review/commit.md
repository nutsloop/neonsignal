# Commit Planning (git add -A)

## Files that would be staged by `git add -A`

From the current working tree, `git add -A` would stage these paths:

- `.babelrc`
- `.clang-format`
- `.clangd`
- `.github/`
- `.gitignore`
- `AGENTS.md`
- `AI-threads/`
- `CLAUDE.md`
- `README.md`
- `benchmark/`
- `eslint.config.js`
- `include/`
- `instructions/`
- `meson.build`
- `neonjsx/`
- `package.json`
- `plans/`
- `scripts/`
- `src/`
- `subprojects/`
- `systemd/`
- `themes/`
- `tsconfig.json`
- `www/`

## Exclusion Decision

`AI-threads/` should **not** be committed. It is generated conversational material and not part of the product deliverable. It has been added to `.gitignore` to prevent staging.
