#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Release Build (backend only)
# Builds the C++ executables in release mode without deleting build dir.
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NEONSIGNAL RELEASE BUILD (BACKEND ONLY)"

cd "$NEONSIGNAL_ROOT_DIR"

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
print_success "Release build complete"
print_substep "Install binaries with: scripts/build/install.sh (uses sudo)"
