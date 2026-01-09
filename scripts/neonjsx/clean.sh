#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NeonJSX Clean"

print_step "Removing build artifacts"
if [[ -d "$NEONSIGNAL_NEONJSX_BUILD_DIR" ]]; then
  rm -rf "$NEONSIGNAL_NEONJSX_BUILD_DIR"
  print_success "Removed build directory: ${NEONSIGNAL_NEONJSX_BUILD_DIR}"
else
  print_substep "Build directory does not exist"
fi

print_success "NeonJSX clean complete"
