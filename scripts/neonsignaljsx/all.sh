#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS]

NeonSignalJSX Pipeline Orchestrator

Options:
  --clean     Clean before building
  --help      Show this help message

Examples:
  $(basename "$0")              # Build only
  $(basename "$0") --clean      # Clean + build
EOF
}

DO_CLEAN=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --clean)
      DO_CLEAN=true
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

print_header "NeonSignalJSX Pipeline"

START_TIME=$(date +%s)

# Step 1: Clean (if requested)
if [[ "$DO_CLEAN" == true ]]; then
  print_step "Cleaning artifacts"
  "$NEONSIGNAL_NEONSIGNALJSX_CLEAN_SCRIPT"
  echo ""
fi

# Step 2: Build
print_step "Building application"
"$NEONSIGNAL_NEONSIGNALJSX_BUILD_SCRIPT"
echo ""

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

print_header "Pipeline Complete"
print_success "Total time: ${ELAPSED}s"
print_substep "Source: ${NEONSIGNAL_NEONSIGNALJSX_SOURCE_DIR}"
print_substep "Build: ${NEONSIGNAL_NEONSIGNALJSX_BUILD_DIR}"
print_substep "Public: ${NEONSIGNAL_NEONSIGNALJSX_PUBLIC_DIR}"
