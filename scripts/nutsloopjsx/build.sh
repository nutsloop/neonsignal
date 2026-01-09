#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NutsloopJSX Build"

print_step "Creating directories"
mkdir -p "$NEONSIGNAL_NUTSLOOPJSX_BUILD_DIR" "$NEONSIGNAL_NUTSLOOPJSX_PUBLIC_DIR" "$NEONSIGNAL_NEONJSX_BUILD_DIR"

print_step "Transpiling NeonJSX runtime"
"$NEONSIGNAL_NEONJSX_BUILD_SCRIPT"

print_step "Transpiling NutsloopJSX source"
print_substep "Source: ${NEONSIGNAL_NUTSLOOPJSX_SOURCE_DIR}"
print_substep "Output: ${NEONSIGNAL_NUTSLOOPJSX_BUILD_DIR}"
npx babel "$NEONSIGNAL_NUTSLOOPJSX_SOURCE_DIR" --extensions .ts,.tsx --out-dir "$NEONSIGNAL_NUTSLOOPJSX_BUILD_DIR"
print_success "Source transpiled"

print_step "Bundling application"
print_substep "Output: ${NEONSIGNAL_NUTSLOOPJSX_PUBLIC_DIR}/app.js"
npx esbuild "$NEONSIGNAL_NUTSLOOPJSX_BUILD_DIR/main.js" \
  --bundle \
  --minify \
  --sourcemap \
  --format=iife \
  --global-name=NutsLoop \
  --outfile="$NEONSIGNAL_NUTSLOOPJSX_PUBLIC_DIR/app.js"
print_success "Bundle created"

print_step "Copying static assets"
cp "$NEONSIGNAL_NUTSLOOPJSX_SOURCE_DIR/index.html" "$NEONSIGNAL_NUTSLOOPJSX_PUBLIC_DIR/index.html"
print_substep "Copied index.html"

if [[ -d "$NEONSIGNAL_NUTSLOOPJSX_SOURCE_DIR/static" ]]; then
  cp -a "$NEONSIGNAL_NUTSLOOPJSX_SOURCE_DIR/static/." "$NEONSIGNAL_NUTSLOOPJSX_PUBLIC_DIR/"
  print_substep "Copied static directory"
fi
print_success "Static assets copied"

print_success "NutsloopJSX build complete -> ${NEONSIGNAL_NUTSLOOPJSX_PUBLIC_DIR}"
