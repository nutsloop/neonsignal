#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "Default VHost Build"

print_step "Creating public directory"
mkdir -p "$NEONSIGNAL_DEFAULT_PUBLIC_DIR"

print_step "Syncing static assets"
print_substep "Source: ${NEONSIGNAL_DEFAULT_SOURCE_DIR}"
print_substep "Destination: ${NEONSIGNAL_DEFAULT_PUBLIC_DIR}"
rsync --archive --delete --exclude='*.sh' "${NEONSIGNAL_DEFAULT_SOURCE_DIR}/" "${NEONSIGNAL_DEFAULT_PUBLIC_DIR}/"
print_success "Static assets synced"

print_success "Default vhost build complete -> ${NEONSIGNAL_DEFAULT_PUBLIC_DIR}"
