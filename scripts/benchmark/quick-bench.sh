#!/usr/bin/env bash
# Quick benchmark for rapid testing
# Usage: ./quick-bench.sh [url-path]

set -euo pipefail

HOST="https://10.0.0.10:9443"
URL_PATH="${1:-/index.html}"

CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
DIM='\033[2m'
BOLD='\033[1m'
RESET='\033[0m'

print_header() {
  echo ""
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo -e "${CYAN}${BOLD}  ▶︎ NeonSignal Quick Benchmark${RESET}"
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo ""
}

print_step() {
  echo -e "${CYAN}▶︎${RESET} ${BOLD}$1${RESET}"
}

print_substep() {
  echo -e "  ${DIM}»${RESET} $1"
}

print_header
print_step "Target: ${HOST}${URL_PATH}"
echo -e "${DIM}───────────────────────────────────────────────────────────────${RESET}"

# Quick 5-second test
h2load -n 5000 -c 100 -t 4 -D 5 "$HOST$URL_PATH"

echo ""
print_step "Tips"
print_substep "To test cache: ./scripts/quick-bench.sh /index.html"
print_substep "To test API: ./scripts/quick-bench.sh /api/stats"
