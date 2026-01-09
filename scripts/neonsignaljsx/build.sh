#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NeonSignalJSX Build"

print_step "Creating directories"
mkdir -p "$NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR" "$NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR" "$NEONSIGNAL_NEONJSX_BUILD_DIR"

print_step "Transpiling NeonJSX runtime"
"$NEONSIGNAL_NEONJSX_BUILD_SCRIPT"

print_step "Transpiling NeonSignalJSX source"
print_substep "Source: ${NEONSIGNAL_NEONSIGNALJSX_SOURCE_DIR}"
print_substep "Output: ${NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR}"
npx babel "$NEONSIGNAL_NEONSIGNALJSX_SOURCE_DIR" --extensions .ts,.tsx --out-dir "$NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR"
print_success "Source transpiled"

print_step "Bundling application"
print_substep "Output: ${NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR}/app.js"
npx esbuild "$NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR/main.js" \
  --bundle \
  --minify \
  --sourcemap \
  --format=iife \
  --global-name=NeonApp \
  --outfile="$NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR/app.js"
print_success "Bundle created"

print_step "Copying static assets"
if [[ -d "$NEONSIGNAL_NEONSIGNALJSX_SOURCE_DIR/static" ]]; then
  cp -r "$NEONSIGNAL_NEONSIGNALJSX_SOURCE_DIR/static/." "$NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR/"
  print_substep "Copied static directory"
fi
print_success "Static assets copied"

print_success "NeonSignalJSX build complete -> ${NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR}"
