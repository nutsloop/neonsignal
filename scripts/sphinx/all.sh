#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS]

NeonSignal Sphinx Pipeline Orchestrator

Options:
  --clean     Clean build/public before building (keeps venv)
  --fresh     Full reset: clean everything including venv
  --no-dynamics   Skip dynamic content generation
  --deploy-to=<path>  Override public deployment directory
  --help      Show this help message

Examples:
  $(basename "$0")              # Quick rebuild (setup + dynamics + build)
  $(basename "$0") --clean      # Clean rebuild (clean + setup + dynamics + build)
  $(basename "$0") --fresh      # Full reset (clean --venv + setup + dynamics + build)
  $(basename "$0") --no-dynamics   # Rebuild without refreshing dynamic content
  $(basename "$0") --deploy-to=/path/to/site/book
EOF
}

DO_CLEAN=false
DO_FRESH=false
DO_DYNAMICS=true
DEPLOY_TO=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --clean)
      DO_CLEAN=true
      shift
      ;;
    --fresh)
      DO_FRESH=true
      shift
      ;;
    --no-dynamics)
      DO_DYNAMICS=false
      shift
      ;;
    --deploy-to=*)
      DEPLOY_TO="${1#*=}"
      shift
      ;;
    --deploy-to)
      if [[ $# -lt 2 ]]; then
        print_error "Missing value for --deploy-to"
        usage
        exit 1
      fi
      DEPLOY_TO="$2"
      shift 2
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

print_header "NeonSignal Sphinx Pipeline"

START_TIME=$(date +%s)

# Step 1: Clean (if requested)
if [[ "$DO_FRESH" == true ]]; then
  print_step "Cleaning everything (fresh start)"
  if [[ -n "$DEPLOY_TO" ]]; then
    "$NEONSIGNAL_SPHINX_CLEAN_SCRIPT" --venv --remove-from="$DEPLOY_TO"
  else
    "$NEONSIGNAL_SPHINX_CLEAN_SCRIPT" --venv
  fi
  echo ""
elif [[ "$DO_CLEAN" == true ]]; then
  print_step "Cleaning build artifacts"
  if [[ -n "$DEPLOY_TO" ]]; then
    "$NEONSIGNAL_SPHINX_CLEAN_SCRIPT" --remove-from="$DEPLOY_TO"
  else
    "$NEONSIGNAL_SPHINX_CLEAN_SCRIPT"
  fi
  echo ""
fi

# Step 2: Setup
print_step "Setting up environment"
"$NEONSIGNAL_SPHINX_SETUP_SCRIPT"
echo ""

# Step 3: Dynamics (if enabled)
if [[ "$DO_DYNAMICS" == true ]]; then
  print_step "Generating dynamic content"
  "$NEONSIGNAL_SPHINX_DYNAMICS_SCRIPT"
  echo ""
else
  print_substep "Skipping dynamic content generation"
fi

# Step 4: Build
print_step "Building and deploying"
if [[ -n "$DEPLOY_TO" ]]; then
  "$NEONSIGNAL_SPHINX_BUILD_SCRIPT" --deploy-to="$DEPLOY_TO"
else
  "$NEONSIGNAL_SPHINX_BUILD_SCRIPT"
fi
echo ""

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

PUBLIC_DIR="$NEONSIGNAL_SPHINX_PUBLIC_DIR"
PUBLIC_ENTRY="$NEONSIGNAL_SPHINX_PUBLIC_ENTRY"
if [[ -n "$DEPLOY_TO" ]]; then
  PUBLIC_DIR="$DEPLOY_TO"
  PUBLIC_ENTRY="$(dirname "$PUBLIC_DIR")/neonsignal-book.html"
fi

print_header "Pipeline Complete"
print_success "Total time: ${ELAPSED}s"
print_substep "Book source: ${NEONSIGNAL_SPHINX_BOOK_DIR}"
print_substep "Build output: ${NEONSIGNAL_SPHINX_BUILD_DIR}"
print_substep "Public site: ${PUBLIC_DIR}"
print_substep "Entry page: ${PUBLIC_ENTRY}"
