#!/usr/bin/env bash
set -euo pipefail

# ═══════════════════════════════════════════════════════════════════════════════
# NeonSignal Production Rewrite Script
# ═══════════════════════════════════════════════════════════════════════════════
#
# Transforms the codebase for production deployment:
#   - Rewrites .host domains to .com
#   - Rewrites development IPs to production IPs
#
# Usage:
#   ./scripts/deployment/rewrite.sh              # Full rewrite (domains + IPs)
#   ./scripts/deployment/rewrite.sh --dry-run    # Preview changes
#   ./scripts/deployment/rewrite.sh --domains-only
#   ./scripts/deployment/rewrite.sh --ips-only
#   ./scripts/deployment/rewrite.sh --verify     # Check no .host remains

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

# ─────────────────────────────────────────────────────────────────────────────────
# Configuration
# ─────────────────────────────────────────────────────────────────────────────────

# Domain mappings (order matters: most specific first)
DOMAIN_MAPPINGS=(
  "neonsignal.nutsloop.host:neonsignal.nutsloop.com"
  "simonedelpopolo.host:simonedelpopolo.com"
  "nutsloop.host:nutsloop.com"
)

# Files/directories to exclude from domain rewriting
EXCLUSIONS=(
  '--glob' '!**/.git/**'
  '--glob' '!include/neonsignal/neonsignal.h++'
  '--glob' '!scripts/certificates/**'
  '--glob' '!scripts/deployment/**'
  '--glob' '!www/book/**'
  '--glob' '!AI-threads/**'
  '--glob' '!instructions/**'
  '--glob' '!AGENTS.md'
  '--glob' '!CLAUDE.md'
  '--glob' '!README.md'
  '--glob' '!plans/**'
)

# Paths to check for IP rewriting
IP_REWRITE_PATHS=(
  "systemd"
  "scripts/benchmark"
)

# Runtime flags
DRY_RUN=false
DOMAINS_ONLY=false
IPS_ONLY=false
VERIFY_ONLY=false

# ─────────────────────────────────────────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────────────────────────────────────────

check_ripgrep() {
  if ! command -v rg >/dev/null 2>&1; then
    print_error "ripgrep (rg) is required"
    print_substep "Install with: sudo dnf install ripgrep"
    exit 1
  fi
}

check_python() {
  if ! command -v python3 >/dev/null 2>&1; then
    print_error "python3 is required"
    exit 1
  fi
}

# ─────────────────────────────────────────────────────────────────────────────────
# Domain Rewriting
# ─────────────────────────────────────────────────────────────────────────────────

# Build regex pattern for all .host domains
build_domain_pattern() {
  local pattern=""
  for mapping in "${DOMAIN_MAPPINGS[@]}"; do
    local from="${mapping%%:*}"
    # Escape dots for regex
    local escaped="${from//./\\.}"
    if [[ -n "$pattern" ]]; then
      pattern="${pattern}|${escaped}"
    else
      pattern="${escaped}"
    fi
  done
  echo "$pattern"
}

# Find files containing .host domains
collect_domain_files() {
  local search_dir="${1:-.}"
  local pattern
  pattern=$(build_domain_pattern)

  rg -l --no-messages --text \
    "${EXCLUSIONS[@]}" \
    "$pattern" "$search_dir" 2>/dev/null || true
}

# Verify no .host domains remain
verify_domains() {
  local search_dir="${1:-.}"
  local -a remaining=()
  mapfile -t remaining < <(collect_domain_files "$search_dir")

  if [[ ${#remaining[@]} -eq 0 ]]; then
    print_success "No .host domains found"
    return 0
  else
    print_error "Domain rewrite incomplete - .host domains remain:"
    for file in "${remaining[@]}"; do
      echo -e "  ${RED}│${RESET} $file"
    done
    return 1
  fi
}

# Rewrite .host domains to .com
rewrite_domains() {
  local search_dir="${1:-.}"
  local -a files=()
  mapfile -t files < <(collect_domain_files "$search_dir")

  if [[ ${#files[@]} -eq 0 ]]; then
    print_substep "No domain rewrites needed"
    return 0
  fi

  print_substep "Found ${#files[@]} file(s) with .host domains"

  if [[ "$DRY_RUN" == "true" ]]; then
    print_warning "DRY-RUN: Would rewrite:"
    for file in "${files[@]}"; do
      echo -e "  ${DIM}│${RESET} $file"
    done
    return 0
  fi

  print_substep "Rewriting: ${YELLOW}.host${RESET} ${DIM}→${RESET} ${GREEN}.com${RESET}"

  # Build Python replacement tuples
  local py_replacements=""
  for mapping in "${DOMAIN_MAPPINGS[@]}"; do
    local from="${mapping%%:*}"
    local to="${mapping##*:}"
    py_replacements="${py_replacements}    (b\"${from}\", b\"${to}\"),
"
  done

  python3 - "${files[@]}" <<PY
import os
import sys

paths = sys.argv[1:]
replacements = (
${py_replacements})

for path in paths:
    try:
        stat = os.stat(path)
        with open(path, "rb") as handle:
            data = handle.read()
        updated = data
        for old, new in replacements:
            updated = updated.replace(old, new)
        if updated != data:
            with open(path, "wb") as handle:
                handle.write(updated)
            os.chmod(path, stat.st_mode)
    except OSError as exc:
        print(f"Failed to update {path}: {exc}", file=sys.stderr)
        sys.exit(1)
PY

  # Verify
  local -a after=()
  mapfile -t after < <(collect_domain_files "$search_dir")
  if [[ ${#after[@]} -eq 0 ]]; then
    print_success "Domains rewritten (${#files[@]} files)"
  else
    print_error "Some .host domains remain: ${#after[@]} file(s)"
    for file in "${after[@]}"; do
      echo -e "  ${RED}│${RESET} $file"
    done
    exit 1
  fi
}

# ─────────────────────────────────────────────────────────────────────────────────
# IP Rewriting
# ─────────────────────────────────────────────────────────────────────────────────

# Find files containing dev IP
collect_ip_files() {
  local search_dir="${1:-.}"
  local -a search_paths=()

  for path in "${IP_REWRITE_PATHS[@]}"; do
    if [[ -e "${search_dir}/${path}" ]]; then
      search_paths+=("${search_dir}/${path}")
    fi
  done

  if [[ ${#search_paths[@]} -eq 0 ]]; then
    return
  fi

  local escaped_ip="${NEONSIGNAL_DEV_IP//./\\.}"
  rg -l --no-messages --text "$escaped_ip" "${search_paths[@]}" 2>/dev/null || true
}

# Rewrite dev IP to prod IP
rewrite_ips() {
  local search_dir="${1:-.}"
  local -a files=()
  mapfile -t files < <(collect_ip_files "$search_dir")

  if [[ ${#files[@]} -eq 0 ]]; then
    print_substep "No IP rewrites needed"
    return 0
  fi

  print_substep "Found ${#files[@]} file(s) with dev IP"

  if [[ "$DRY_RUN" == "true" ]]; then
    print_warning "DRY-RUN: Would rewrite:"
    for file in "${files[@]}"; do
      echo -e "  ${DIM}│${RESET} $file"
    done
    return 0
  fi

  print_substep "Rewriting: ${YELLOW}${NEONSIGNAL_DEV_IP}${RESET} ${DIM}→${RESET} ${GREEN}${NEONSIGNAL_PROD_IP}${RESET}"

  python3 - "${files[@]}" <<PY
import os
import sys

paths = sys.argv[1:]
old = b"${NEONSIGNAL_DEV_IP}"
new = b"${NEONSIGNAL_PROD_IP}"

for path in paths:
    try:
        stat = os.stat(path)
        with open(path, "rb") as handle:
            data = handle.read()
        updated = data.replace(old, new)
        if updated != data:
            with open(path, "wb") as handle:
                handle.write(updated)
            os.chmod(path, stat.st_mode)
    except OSError as exc:
        print(f"Failed to update {path}: {exc}", file=sys.stderr)
        sys.exit(1)
PY

  # Verify
  local -a after=()
  mapfile -t after < <(collect_ip_files "$search_dir")
  if [[ ${#after[@]} -eq 0 ]]; then
    print_success "IPs rewritten (${#files[@]} files)"
  else
    print_warning "Some dev IPs remain: ${#after[@]} file(s)"
  fi
}

# ─────────────────────────────────────────────────────────────────────────────────
# Help
# ─────────────────────────────────────────────────────────────────────────────────

show_help() {
  print_header "NeonSignal Production Rewrite"

  echo -e "${BOLD}Usage:${RESET}"
  echo -e "  ${CYAN}rewrite.sh${RESET}                 Run all rewrites (domains + IPs)"
  echo -e "  ${CYAN}rewrite.sh${RESET} ${YELLOW}--dry-run${RESET}      Preview changes without modifying"
  echo -e "  ${CYAN}rewrite.sh${RESET} ${YELLOW}--domains-only${RESET} Only rewrite domains"
  echo -e "  ${CYAN}rewrite.sh${RESET} ${YELLOW}--ips-only${RESET}     Only rewrite IPs"
  echo -e "  ${CYAN}rewrite.sh${RESET} ${YELLOW}--verify${RESET}       Check no .host domains remain"
  echo ""

  echo -e "${BOLD}Domain Mappings:${RESET}"
  for mapping in "${DOMAIN_MAPPINGS[@]}"; do
    local from="${mapping%%:*}"
    local to="${mapping##*:}"
    echo -e "  ${DIM}│${RESET} ${YELLOW}${from}${RESET} ${DIM}→${RESET} ${GREEN}${to}${RESET}"
  done
  echo ""

  echo -e "${BOLD}IP Mapping:${RESET}"
  echo -e "  ${DIM}│${RESET} ${YELLOW}${NEONSIGNAL_DEV_IP}${RESET} ${DIM}→${RESET} ${GREEN}${NEONSIGNAL_PROD_IP}${RESET}"
  echo ""

  echo -e "${BOLD}Excluded from rewrite:${RESET}"
  echo -e "  ${DIM}│${RESET} .git/"
  echo -e "  ${DIM}│${RESET} include/neonsignal/neonsignal.h++"
  echo -e "  ${DIM}│${RESET} scripts/certificates/"
  echo -e "  ${DIM}│${RESET} scripts/deployment/"
  echo -e "  ${DIM}│${RESET} www/book/source/part4-operations/index.md"
  echo -e "  ${DIM}│${RESET} www/book/source/part2-architecture/virtual-hosting.md"
  echo -e "  ${DIM}│${RESET} AGENTS.md"
  echo -e "  ${DIM}│${RESET} plans/"
  echo ""
}

# ─────────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────────

main() {
  # Parse arguments
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --dry-run)
        DRY_RUN=true
        shift
        ;;
      --domains-only)
        DOMAINS_ONLY=true
        shift
        ;;
      --ips-only)
        IPS_ONLY=true
        shift
        ;;
      --verify)
        VERIFY_ONLY=true
        shift
        ;;
      help|--help|-h)
        show_help
        exit 0
        ;;
      *)
        print_error "Unknown option: $1"
        echo ""
        show_help
        exit 1
        ;;
    esac
  done

  # Must run from repo root
  if [[ ! -f "$NEONSIGNAL_ROOT_DIR/.gitignore" ]]; then
    print_error "Must run from repository root"
    exit 1
  fi

  cd "$NEONSIGNAL_ROOT_DIR"

  # Check dependencies
  check_ripgrep
  check_python

  # Verify only mode
  if [[ "$VERIFY_ONLY" == "true" ]]; then
    print_header "NeonSignal Domain Verification"
    verify_domains "."
    exit $?
  fi

  # Header
  if [[ "$DRY_RUN" == "true" ]]; then
    print_header "NeonSignal Production Rewrite (DRY-RUN)"
  else
    print_header "NeonSignal Production Rewrite"
  fi

  # Run rewrites
  if [[ "$IPS_ONLY" != "true" ]]; then
    print_step "Domain rewrite"
    rewrite_domains "."
    echo ""
  fi

  if [[ "$DOMAINS_ONLY" != "true" ]]; then
    print_step "IP address rewrite"
    rewrite_ips "."
    echo ""
  fi

  print_separator
  if [[ "$DRY_RUN" == "true" ]]; then
    echo -e "${YELLOW}${BOLD}DRY-RUN complete - no files modified${RESET}"
  else
    echo -e "${GREEN}${BOLD}Rewrite complete${RESET}"
  fi
  echo ""
}

main "$@"
