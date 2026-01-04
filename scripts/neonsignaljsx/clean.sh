#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NeonSignalJSX Clean"

print_step "Removing build artifacts"
if [[ -d "$NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR" ]]; then
  rm -rf "$NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR"
  print_success "Removed build directory: ${NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR}"
else
  print_substep "Build directory does not exist"
fi

print_step "Removing public deployment"
if [[ -d "$NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR" ]]; then
  rm -rf "$NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR"
  print_success "Removed public directory: ${NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR}"
else
  print_substep "Public directory does not exist"
fi

print_success "NeonSignalJSX clean complete"
