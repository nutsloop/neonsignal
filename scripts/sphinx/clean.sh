#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NeonSignal Sphinx Clean"

print_step "Removing build artifacts"
if [[ -d "$NEONSIGNAL_SPHINX_BUILD_DIR" ]]; then
  rm -rf "$NEONSIGNAL_SPHINX_BUILD_DIR"
  print_success "Removed build directory: ${NEONSIGNAL_SPHINX_BUILD_DIR}"
else
  print_substep "Build directory does not exist"
fi

print_step "Removing public deployment"
if [[ -d "$NEONSIGNAL_SPHINX_PUBLIC_DIR" ]]; then
  rm -rf "$NEONSIGNAL_SPHINX_PUBLIC_DIR"
  print_success "Removed public directory: ${NEONSIGNAL_SPHINX_PUBLIC_DIR}"
else
  print_substep "Public directory does not exist"
fi

if [[ -f "$NEONSIGNAL_SPHINX_PUBLIC_ENTRY" ]]; then
  rm -f "$NEONSIGNAL_SPHINX_PUBLIC_ENTRY"
  print_success "Removed public entry file: ${NEONSIGNAL_SPHINX_PUBLIC_ENTRY}"
else
  print_substep "Public entry file does not exist"
fi

if [[ "${1:-}" == "--venv" ]]; then
  print_step "Removing virtual environment"
  if [[ -d "$NEONSIGNAL_SPHINX_VENV_DIR" ]]; then
    rm -rf "$NEONSIGNAL_SPHINX_VENV_DIR"
    print_success "Removed virtual environment: ${NEONSIGNAL_SPHINX_VENV_DIR}"
  else
    print_substep "Virtual environment does not exist"
  fi
fi

print_success "Sphinx clean complete"
