#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Release Build Script (no wipe)
# Builds backend + frontends in release mode without deleting build dir.
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# ─────────────────────────────────────────────────────────────────────────────
# Synthwave Color Palette
# ─────────────────────────────────────────────────────────────────────────────
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
DIM='\033[2m'
BOLD='\033[1m'
RESET='\033[0m'

# ─────────────────────────────────────────────────────────────────────────────
# Helper Functions
# ─────────────────────────────────────────────────────────────────────────────
print_header() {
  local title="$1"
  echo ""
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo -e "${CYAN}${BOLD}  ▶︎ ${title}${RESET}"
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

print_header "NEONSIGNAL RELEASE BUILD"

cd "$PROJECT_DIR"

print_step "Configuring Meson for release build"
if [ -d "build" ]; then
  print_substep "Reconfiguring existing build"
  meson configure build -Dbuildtype=release -Doptimization=2 -Ddebug=false
else
  print_substep "Setting up new build"
  meson setup build --buildtype=release -Doptimization=2 -Ddebug=false
fi
print_success "Meson configured"

print_separator
print_step "Clean rebuild"
print_substep "Ensures all targets use release settings"
meson compile -C build --clean
print_success "Build cleaned"

print_separator
print_step "Compiling C++ ${DIM}(release -O2)${RESET}"
meson compile -C build
print_success "Compilation complete"

print_separator
print_step "Stripping debug symbols"
strip build/src/neonsignal build/src/neonsignal_redirect
print_success "Binaries stripped"

echo ""
print_substep "Binary sizes:"
ls -lh build/src/neonsignal build/src/neonsignal_redirect | while read -r line; do
  echo -e "    ${DIM}│${RESET} ${CYAN}${line}${RESET}"
done

print_separator
print_step "Building frontends"
print_substep "Installing npm dependencies"
npm install

print_substep "Cleaning old builds"
npm run clean:neonjsx
npm run clean:neonsignal
npm run clean:nutsloop
npm run clean:simonedelpopolo
npm run clean:_default

print_substep "Building all sites"
npm run build:neonjsx
npm run build:neonsignal
npm run build:nutsloop
npm run build:simonedelpopolo
npm run build:_default
print_success "Frontends built"

print_separator
print_success "Release build complete"
