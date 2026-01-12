#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal clang-format Script
# Formats C++ source and header files under src/ and include/
#
# Usage:
#   ./scripts/build/clang-format.sh --check   # check only
#   ./scripts/build/clang-format.sh --fix     # check and fix
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

usage() {
  echo ""
  echo "Usage:"
  echo "  $0 --check   Check formatting only"
  echo "  $0 --fix     Check and fix formatting"
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

if ! command -v clang-format >/dev/null 2>&1; then
  print_error "clang-format not found in PATH"
  exit 1
fi

print_header "NEONSIGNAL CLANG-FORMAT"

cd "$NEONSIGNAL_ROOT_DIR"

format_dirs=("$NEONSIGNAL_ROOT_DIR/src" "$NEONSIGNAL_ROOT_DIR/include")
files=()

print_step "Collecting C++ files"
for dir in "${format_dirs[@]}"; do
  if [[ ! -d "$dir" ]]; then
    print_warning "Missing directory: $dir"
    continue
  fi
  mapfile -t found < <(find "$dir" -type f \( -name '*.c++' -o -name '*.h++' \))
  files+=("${found[@]}")
  print_substep "${#found[@]} files in $dir"
done

if [[ ${#files[@]} -eq 0 ]]; then
  print_warning "No C++ files found to format"
  exit 0
fi

check_format() {
  clang-format --dry-run --Werror "${files[@]}"
}

apply_format() {
  clang-format -i "${files[@]}"
}

if [[ "$mode" == "check" ]]; then
  print_separator
  print_step "Checking formatting"
  check_format
  print_success "Formatting check passed"
  exit 0
fi

print_separator
print_step "Checking formatting"
if ! check_format; then
  print_warning "Formatting issues detected"
fi

print_separator
print_step "Applying clang-format"
apply_format
print_success "Formatting applied"
