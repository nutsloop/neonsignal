#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "SimoneDelPopolo Clean"

print_step "Removing public deployment"
if [[ -d "$NEONSIGNAL_SIMONEDELPOPOLO_PUBLIC_DIR" ]]; then
  rm -rf "$NEONSIGNAL_SIMONEDELPOPOLO_PUBLIC_DIR"
  print_success "Removed public directory: ${NEONSIGNAL_SIMONEDELPOPOLO_PUBLIC_DIR}"
else
  print_substep "Public directory does not exist"
fi

print_success "SimoneDelPopolo clean complete"
