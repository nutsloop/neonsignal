#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NeonJSX Build"

print_step "Creating directories"
mkdir -p "$NEONSIGNAL_NEONJSX_BUILD_DIR"

print_step "Transpiling NeonJSX runtime"
print_substep "Source: ${NEONSIGNAL_NEONJSX_SOURCE_DIR}/runtime.ts"
print_substep "Output: ${NEONSIGNAL_NEONJSX_BUILD_DIR}"
npx babel "$NEONSIGNAL_NEONJSX_SOURCE_DIR/runtime.ts" --extensions .ts --out-dir "$NEONSIGNAL_NEONJSX_BUILD_DIR"
print_success "Runtime transpiled"

print_success "NeonJSX build complete -> ${NEONSIGNAL_NEONJSX_BUILD_DIR}"
