#!/usr/bin/env bash

CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
DIM='\033[2m'
BOLD='\033[1m'
RESET='\033[0m'

ICON_HEADER='✦'
ICON_STEP='➤'
ICON_SUBSTEP='•'
ICON_SUCCESS='✔'
ICON_WARNING='▲'
ICON_ERROR='✖'

print_header() {
  local title="$1"
  echo ""
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo -e "${CYAN}${BOLD}  ${ICON_HEADER} ${title}${RESET}"
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo ""
}

print_separator() {
  echo -e "${DIM}───────────────────────────────────────────────────────────────${RESET}"
}

print_step() {
  echo -e "${CYAN}${ICON_STEP}${RESET} ${BOLD}$1${RESET}"
}

print_substep() {
  echo -e "  ${DIM}${ICON_SUBSTEP}${RESET} $1"
}

print_success() {
  echo -e "${GREEN}${ICON_SUCCESS}${RESET} $1"
}

print_warning() {
  echo -e "${YELLOW}${ICON_WARNING}${RESET} $1"
}

print_error() {
  echo -e "${RED}${ICON_ERROR}${RESET} $1" >&2
}
