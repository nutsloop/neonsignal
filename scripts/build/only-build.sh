#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Build Script (macOS) - Compile only.
#
# Usage:
#   ./scripts/build/on-build.sh
# ═══════════════════════════════════════════════════════════════════════════

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

OS_NAME="$(uname -s)"

command_exists() {
  command -v "$1" >/dev/null 2>&1
}

brew_has_formula() {
  command_exists brew && brew list --versions "$1" >/dev/null 2>&1
}

ensure_command_or_brew() {
  local cmd="$1"
  local formula="$2"

  if command_exists "$cmd"; then
    return 0
  fi

  if command_exists brew; then
    if brew_has_formula "$formula"; then
      print_error "Command '${cmd}' not found in PATH but brew formula '${formula}' is installed."
      print_substep "Check your PATH or run: eval \"\\$(brew shellenv)\""
    else
      print_error "Missing command '${cmd}'."
      print_substep "Install with: brew install ${formula}"
    fi
    return 1
  fi

  print_error "Missing command '${cmd}'."
  print_substep "Install it or add it to PATH."
  return 1
}

preflight_checks() {
  if [[ "${OS_NAME}" != "Darwin" ]]; then
    print_error "This build script is macOS-only. Detected: ${OS_NAME}"
    exit 1
  fi

  local missing=false
  ensure_command_or_brew meson meson || missing=true
  ensure_command_or_brew ninja ninja || missing=true

  if [[ "${missing}" == "true" ]]; then
    print_error "Missing required build tools. Aborting."
    exit 1
  fi
}

show_help() {
  print_header "NEONSIGNAL BUILD (macOS, COMPILE ONLY)"
  echo -e "${BOLD}Usage:${RESET}"
  echo -e "  ${CYAN}./scripts/build/on-build.sh${RESET}"
  echo ""
}

# ─────────────────────────────────────────────────────────────────────────────
# Parse Arguments
# ─────────────────────────────────────────────────────────────────────────────
for arg in "$@"; do
  case "$arg" in
    --help|-h)
      show_help
      exit 0
      ;;
    *)
      print_error "Unknown option: $arg"
      show_help
      exit 1
      ;;
  esac
done

# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────
print_header "NEONSIGNAL BUILD (macOS, COMPILE ONLY)"
preflight_checks

cd "$NEONSIGNAL_ROOT_DIR"

if [[ ! -d "build" ]]; then
  print_error "Build directory not found."
  print_substep "Run ./scripts/build/all.sh or meson setup build first."
  exit 1
fi

print_separator
print_step "Compiling C++"
meson compile -C build
print_success "Compilation complete"

echo ""
print_substep "Binaries:"
ls -lh build/src/neonsignal build/src/neonsignal_redirect | while read -r line; do
  echo -e "    ${DIM}│${RESET} ${CYAN}${line}${RESET}"
done
