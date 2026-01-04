#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "ESLint Fix"

print_step "Fixing TypeScript/TSX files"
print_substep "neonjsx/"
print_substep "www/neonsignaljsx/"
print_substep "www/nutsloopjsx/"

npx eslint --fix \
  "$NEONSIGNAL_NEONJSX_SOURCE_DIR/**/*.{ts,tsx}" \
  "$NEONSIGNAL_NEONSIGNALJSX_SOURCE_DIR/**/*.{ts,tsx}" \
  "$NEONSIGNAL_NUTSLOOPJSX_SOURCE_DIR/**/*.{ts,tsx}"

print_success "Lint fix complete"
