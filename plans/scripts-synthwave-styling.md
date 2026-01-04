# Plan: Synthwave Styling for NeonSignal Scripts

## Request

Apply synthwave/wasteland theme to the following scripts:

- `scripts/rsync_deploy.sh`
- `scripts/install.sh`
- `scripts/letsencrypt.sh`
- `scripts/re-neonsignal-all.sh`

Style requirements:
- Use colorful ANSI escape codes (cyan, magenta, yellow - synthwave palette)
- Replace emoji with Unicode characters like `▶︎` (U+25B6 U+FE0E)
- Keep functional output readable but atmospheric
- Match the aesthetic of the NeonSignal project

## Color Palette

```bash
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
DIM='\033[2m'
BOLD='\033[1m'
RESET='\033[0m'
```

## Unicode Characters

| Symbol | Name | Use |
|--------|------|-----|
| `▶︎` | Black right-pointing triangle | Action start |
| `◆` | Black diamond | Status marker |
| `│` | Box drawing vertical | Progress lines |
| `─` | Box drawing horizontal | Separators |
| `═` | Box drawing double horizontal | Headers |
| `✓` | Check mark | Success (in text, not as prefix) |
| `✗` | Ballot X | Failure |
| `»` | Right double angle | Sub-steps |

## Helper Functions Template

```bash
print_header() {
  echo ""
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo -e "${CYAN}${BOLD}  ▶︎ SCRIPT NAME${RESET}"
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo ""
}

print_step() {
  echo -e "${CYAN}▶︎${RESET} ${BOLD}$1${RESET}"
}

print_substep() {
  echo -e "  ${DIM}»${RESET} $1"
}

print_success() {
  echo -e "${GREEN}◆${RESET} $1"
}

print_error() {
  echo -e "${RED}✗${RESET} $1" >&2
}

print_warning() {
  echo -e "${YELLOW}◆${RESET} $1"
}

print_separator() {
  echo -e "${DIM}───────────────────────────────────────────────────────────────${RESET}"
}
```

## Implementation Example

Replace plain echo statements with colored versions:

```bash
# Before
echo "Syncing to ${REMOTE} ..."

# After
echo -e "${CYAN}▶︎${RESET} ${BOLD}Syncing to${RESET} ${MAGENTA}${REMOTE}${RESET}"
```

## Backups

- `scripts/rsync_deploy.sh.bak`
- `scripts/install.sh.bak`
- `scripts/letsencrypt.sh.bak`
- `scripts/re-neonsignal-all.sh.bak`

## Files Modified

| Script | Changes |
|--------|---------|
| `rsync_deploy.sh` | First script styled; added color palette, helper functions, remote rebuild execution |
| `install.sh` | Replace emoji with Unicode, add color palette, use helper functions |
| `letsencrypt.sh` | Update existing log_* functions to use synthwave style |
| `re-neonsignal-all.sh` | Full synthwave overhaul: color palette, helper functions, replace all emoji with Unicode equivalents, structured output with separators |
| `neonsignal-sphinx-build.sh` | Added synthwave palette, header/step helpers, and structured output for build and deploy phases |
| `neonsignal-sphinx-setup.sh` | Added synthwave palette, header/step helpers, and structured setup logs for venv/theme/doc stubs |
| `benchmark.sh` | Switched to synthwave palette with header/step helpers, replaced emoji with glyph markers, structured test sections |
| `quick-bench.sh` | Added synthwave palette, header/step helpers, and formatted tips output |
