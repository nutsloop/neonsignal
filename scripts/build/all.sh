#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Full Rebuild & Deploy Script
# Builds frontend + backend in release mode, then deploys to /usr/local/bin
#
# Usage:
#   ./scripts/build/all.sh                          # Production (rewrites domains, LE certs)
#   ./scripts/build/all.sh --local                  # Development (keeps .host, local certs)
#   ./scripts/build/all.sh --delete-build           # Wipe build/ before configure
#   ./scripts/build/all.sh --reset-db               # Wipe data/ before start
#   ./scripts/build/all.sh --delete-build --reset-db
# ═══════════════════════════════════════════════════════════════════════════

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

DELETE_BUILD=false
LOCAL_MODE=false
RESET_DB=false

# ─────────────────────────────────────────────────────────────────────────────
# Parse Arguments
# ─────────────────────────────────────────────────────────────────────────────
for arg in "$@"; do
  case "$arg" in
    --delete-build)
      DELETE_BUILD=true
      ;;
    --local)
      LOCAL_MODE=true
      ;;
    --reset-db)
      RESET_DB=true
      ;;
  esac
done

# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────
if [[ "$LOCAL_MODE" == "true" ]]; then
  print_header "NEONSIGNAL BUILD & DEPLOY (LOCAL)"
else
  print_header "NEONSIGNAL BUILD & DEPLOY (PRODUCTION)"
fi

# Stop existing services
print_step "Stopping services"
sudo systemctl stop neonsignal.service neonsignal-redirect.service || true
sudo systemctl disable neonsignal.service neonsignal-redirect.service || true
sudo rm -f /etc/systemd/system/neonsignal.service /etc/systemd/system/neonsignal-redirect.service || true
sudo systemctl daemon-reload || true
print_success "Services stopped"

cd "$NEONSIGNAL_ROOT_DIR"

# Optional DB reset
if [[ "$RESET_DB" == "true" ]]; then
  print_separator
  print_warning "Resetting database directory"
  rm -rf data
  mkdir -p data
  print_success "Database directory reset"
fi

# Rewrite domains and IPs for production (skip in local mode)
if [[ "$LOCAL_MODE" != "true" ]]; then
  print_separator
  print_step "Rewriting domains and IPs for production"
  "$NEONSIGNAL_DEPLOYMENT_REWRITE_SCRIPT"
else
  print_separator
  print_step "Skipping domain/IP rewrite (local mode)"
  print_substep "Keeping .host domains and dev IPs"
fi

# Configure Meson
print_separator
print_step "Configuring Meson for release build"
if [[ "$DELETE_BUILD" == "true" ]]; then
  print_warning "Removing build directory"
  rm -rf build
fi
if [ -d "build" ]; then
  print_substep "Reconfiguring existing build"
  meson configure build -Dbuildtype=release -Doptimization=2 -Ddebug=false
else
  print_substep "Setting up new build"
  meson setup build --buildtype=release -Doptimization=2 -Ddebug=false
fi
print_success "Meson configured"

# Clean rebuild
print_separator
print_step "Clean rebuild"
print_substep "Ensures all targets use release settings"
meson compile -C build --clean
print_success "Build cleaned"

# Compile C++
print_separator
print_step "Compiling C++ ${DIM}(release -O2)${RESET}"
meson compile -C build
print_success "Compilation complete"

# Strip binaries
print_separator
print_step "Stripping debug symbols"
strip build/src/neonsignal build/src/neonsignal_redirect
print_success "Binaries stripped"

echo ""
print_substep "Binary sizes:"
ls -lh build/src/neonsignal build/src/neonsignal_redirect | while read -r line; do
  echo -e "    ${DIM}│${RESET} ${CYAN}${line}${RESET}"
done

# Install systemd services
print_separator
print_step "Installing systemd services"
sudo install -m 0644 systemd/neonsignal.service /etc/systemd/system/neonsignal.service
sudo install -m 0644 systemd/neonsignal-redirect.service /etc/systemd/system/neonsignal-redirect.service
sudo systemctl enable neonsignal.service neonsignal-redirect.service || true
sudo systemctl daemon-reload
print_success "Services installed"

# Run install script (SELinux-aware)
print_separator
print_step "Installing binaries"
"$SCRIPT_DIR/install.sh"

# Build frontends
print_separator
print_step "Building frontends"
print_substep "Clearing npm state"
rm -rf node_modules package-lock.json

print_substep "Installing npm dependencies"
npm install

print_substep "Cleaning old builds"
npm run clean:neonsignal
npm run clean:nutsloop
npm run clean:simonedelpopolo
npm run clean:_default

print_substep "Building all sites"
npm run build:neonsignal
npm run build:nutsloop
npm run build:simonedelpopolo
npm run build:_default
print_success "Frontends built"

# Symlinks
print_separator
print_step "Setting up symlinks"
"$SCRIPT_DIR/../vhost/www-domain-symlink.sh" || true
print_success "Symlinks configured"

# Certificates
print_separator
if [[ "$LOCAL_MODE" == "true" ]]; then
  print_step "Generating local development certificates"
  "$NEONSIGNAL_CERT_ISSUER_SCRIPT" --local generate-all || true
else
  print_step "Starting redirect service for ACME challenge"
  sudo systemctl start neonsignal-redirect.service || true
  sleep 1
  sudo systemctl is-active --quiet neonsignal-redirect.service || \
    print_warning "Redirect service not active; ACME challenge may fail"
  print_success "Redirect service ready"

  print_step "Requesting Let's Encrypt certificates"
  sudo "$NEONSIGNAL_CERT_ISSUER_SCRIPT" --letsencrypt request-all || true
fi
print_success "Certificates processed"

# Start services
print_separator
print_step "Starting services"
sudo systemctl start neonsignal.service neonsignal-redirect.service
print_success "Services started"

# Summary
print_separator
echo ""
if [[ "$LOCAL_MODE" == "true" ]]; then
  echo -e "${GREEN}${BOLD}◆ Build & Deploy complete (LOCAL)${RESET}"
else
  echo -e "${GREEN}${BOLD}◆ Build & Deploy complete (PRODUCTION)${RESET}"
fi
echo ""
print_substep "Services running:"
echo -e "    ${DIM}│${RESET} ${CYAN}neonsignal.service${RESET}"
echo -e "    ${DIM}│${RESET} ${CYAN}neonsignal-redirect.service${RESET}"
echo ""
print_substep "Commands:"
echo -e "    ${DIM}│${RESET} sudo systemctl status neonsignal neonsignal-redirect"
echo -e "    ${DIM}│${RESET} sudo journalctl -u neonsignal -f"
echo ""
