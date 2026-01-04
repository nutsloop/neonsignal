#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Service Reload
# Reloads systemd units and restarts neonsignal services.
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

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

print_success() {
  echo -e "${GREEN}◆${RESET} $1"
}

print_separator() {
  echo -e "${DIM}───────────────────────────────────────────────────────────────${RESET}"
}

print_header "RELOAD NEONSIGNAL SERVICES"

print_step "Reloading systemd daemon"
sudo systemctl daemon-reload
print_success "Daemon reloaded"

print_separator
print_step "Restarting services"
sudo systemctl restart neonsignal.service neonsignal-redirect.service
print_success "Services restarted"

print_separator
print_step "Service status"
sudo systemctl status neonsignal.service neonsignal-redirect.service --no-pager
