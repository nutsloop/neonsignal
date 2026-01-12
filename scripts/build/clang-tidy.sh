#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal clang-tidy Script
# Lints C++ source and header files under src/ and include/
#
# Usage:
#   ./scripts/build/clang-tidy.sh --check   # check only
#   ./scripts/build/clang-tidy.sh --fix     # check and fix
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

usage() {
  echo ""
  echo "Usage:"
  echo "  $0 --check   Check linting only"
  echo "  $0 --fix     Check and fix linting issues"
  echo ""
}

mode=""

if [[ "$#" -ne 1 ]]; then
  print_error "Exactly one flag is required"
  usage
  exit 1
fi

case "$1" in
  --check)
    mode="check"
    ;;
  --fix)
    mode="fix"
    ;;
  *)
    print_error "Unknown argument: $1"
    usage
    exit 1
    ;;
esac

if ! command -v clang-tidy >/dev/null 2>&1; then
  print_error "clang-tidy not found in PATH"
  exit 1
fi

build_dir="$NEONSIGNAL_ROOT_DIR/build"
compile_db="$build_dir/compile_commands.json"

if [[ ! -f "$compile_db" ]]; then
  print_error "Missing compile_commands.json at $compile_db"
  print_substep "Run: meson setup build"
  exit 1
fi

print_header "NEONSIGNAL CLANG-TIDY"

cd "$NEONSIGNAL_ROOT_DIR"

lint_dirs=("$NEONSIGNAL_ROOT_DIR/src" "$NEONSIGNAL_ROOT_DIR/include")
files=()

print_step "Collecting C++ files"
for dir in "${lint_dirs[@]}"; do
  if [[ ! -d "$dir" ]]; then
    print_warning "Missing directory: $dir"
    continue
  fi
  mapfile -t found < <(find "$dir" -type f \( -name '*.c++' -o -name '*.h++' \))
  files+=("${found[@]}")
  print_substep "${#found[@]} files in $dir"
done

if [[ ${#files[@]} -eq 0 ]]; then
  print_warning "No C++ files found to lint"
  exit 0
fi

run_check() {
  local failed=0
  for file in "${files[@]}"; do
    if ! clang-tidy -p "$build_dir" -warnings-as-errors='*' "$file"; then
      failed=1
    fi
  done
  return "$failed"
}

run_fix() {
  for file in "${files[@]}"; do
    clang-tidy -p "$build_dir" -fix -format-style=file "$file"
  done
}

if [[ "$mode" == "check" ]]; then
  print_separator
  print_step "Checking linting"
  if ! run_check; then
    print_error "clang-tidy issues detected"
    exit 1
  fi
  print_success "clang-tidy check passed"
  exit 0
fi

print_separator
print_step "Checking linting"
if ! run_check; then
  print_warning "clang-tidy issues detected"
fi

print_separator
print_step "Applying clang-tidy fixes"
run_fix
print_success "clang-tidy fixes applied"
