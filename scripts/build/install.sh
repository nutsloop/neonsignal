#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Install (SELinux-aware)
# Installs binaries to /usr/local/bin and applies SELinux context.
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "INSTALL NEONSIGNAL BINARIES"

cd "$NEONSIGNAL_ROOT_DIR"

print_step "Installing binaries to /usr/local/bin"
if [ ! -f "build/src/neonsignal" ] || [ ! -f "build/src/neonsignal_redirect" ]; then
  print_error "Missing build outputs. Run scripts/build/all.sh first."
  exit 1
fi

sudo install -m 0755 build/src/neonsignal /usr/local/bin/neonsignal
sudo install -m 0755 build/src/neonsignal_redirect /usr/local/bin/neonsignal_redirect
print_success "Binaries installed"

print_separator
print_step "Applying SELinux context"
sudo chcon -t bin_t /usr/local/bin/neonsignal /usr/local/bin/neonsignal_redirect
if command -v restorecon >/dev/null 2>&1; then
  sudo restorecon -v /usr/local/bin/neonsignal /usr/local/bin/neonsignal_redirect || true
fi
print_success "SELinux context applied"

print_separator
print_success "Install complete"
