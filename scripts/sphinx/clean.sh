#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS]

NeonSignal Sphinx Clean

Options:
  --remove-from=<path>  Override public deployment directory to remove
  --venv                Remove virtual environment
  --help                Show this help message
EOF
}

REMOVE_FROM=""
DO_VENV=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --remove-from=*)
      REMOVE_FROM="${1#*=}"
      shift
      ;;
    --remove-from)
      if [[ $# -lt 2 ]]; then
        print_error "Missing value for --remove-from"
        usage
        exit 1
      fi
      REMOVE_FROM="$2"
      shift 2
      ;;
    --venv)
      DO_VENV=true
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      print_error "Unknown option: $1"
      usage
      exit 1
      ;;
  esac
done

PUBLIC_DIR="$NEONSIGNAL_SPHINX_PUBLIC_DIR"
PUBLIC_ENTRY="$NEONSIGNAL_SPHINX_PUBLIC_ENTRY"
if [[ -n "$REMOVE_FROM" ]]; then
  PUBLIC_DIR="$REMOVE_FROM"
  PUBLIC_ENTRY="$(dirname "$PUBLIC_DIR")/neonsignal-book.html"
fi

print_header "NeonSignal Sphinx Clean"

print_step "Removing build artifacts"
if [[ -d "$NEONSIGNAL_SPHINX_BUILD_DIR" ]]; then
  rm -rf "$NEONSIGNAL_SPHINX_BUILD_DIR"
  print_success "Removed build directory: ${NEONSIGNAL_SPHINX_BUILD_DIR}"
else
  print_substep "Build directory does not exist"
fi

print_step "Removing public deployment"
if [[ -d "$PUBLIC_DIR" ]]; then
  rm -rf "$PUBLIC_DIR"
  print_success "Removed public directory: ${PUBLIC_DIR}"
else
  print_substep "Public directory does not exist"
fi

if [[ -f "$PUBLIC_ENTRY" ]]; then
  rm -f "$PUBLIC_ENTRY"
  print_success "Removed public entry file: ${PUBLIC_ENTRY}"
else
  print_substep "Public entry file does not exist"
fi

if [[ "$DO_VENV" == true ]]; then
  print_step "Removing virtual environment"
  if [[ -d "$NEONSIGNAL_SPHINX_VENV_DIR" ]]; then
    rm -rf "$NEONSIGNAL_SPHINX_VENV_DIR"
    print_success "Removed virtual environment: ${NEONSIGNAL_SPHINX_VENV_DIR}"
  else
    print_substep "Virtual environment does not exist"
  fi
fi

print_success "Sphinx clean complete"
