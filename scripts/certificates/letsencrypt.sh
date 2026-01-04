#!/usr/bin/env bash
set -euo pipefail

# ═══════════════════════════════════════════════════════════════════════════
# Let's Encrypt Certificate Management for NeonSignal
# ═══════════════════════════════════════════════════════════════════════════
#
# Usage:
#   sudo ./scripts/letsencrypt.sh request-all              # Request all configured domains
#   sudo ./scripts/letsencrypt.sh --dry-run request-all    # Test without issuing certs
#   sudo ./scripts/letsencrypt.sh request <name> <domain1> [domain2...]
#   sudo ./scripts/letsencrypt.sh verify                   # Show certificate status
#   sudo ./scripts/letsencrypt.sh renew                    # Renew all certificates
#   sudo ./scripts/letsencrypt.sh install-hook             # Install renewal hook

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "${SCRIPT_DIR}")"

CERTS_DIR="${CERTS_DIR:-${PROJECT_DIR}/certs}"
WEBROOT="${WEBROOT:-${PROJECT_DIR}/acme-challenge}"
EMAIL="${EMAIL:-simonedelpopolo@outlook.com}"
CERTBOT="${CERTBOT:-certbot}"
DRY_RUN="${DRY_RUN:-false}"
CERTBOT_RENEW_BEFORE_SECONDS="${CERTBOT_RENEW_BEFORE_SECONDS:-2592000}"

# ─────────────────────────────────────────────────────────────────────────────
# Configured Domains
# ─────────────────────────────────────────────────────────────────────────────

declare -A DOMAINS=(
  ["simonedelpopolo.com"]="simonedelpopolo.com www.simonedelpopolo.com"
  ["nutsloop.com"]="nutsloop.com www.nutsloop.com"
  ["neonsignal.nutsloop.com"]="neonsignal.nutsloop.com"
)

# ─────────────────────────────────────────────────────────────────────────────
# Synthwave Color Palette
# ─────────────────────────────────────────────────────────────────────────────

CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
DIM='\033[2m'
BOLD='\033[1m'
NC='\033[0m'

# ─────────────────────────────────────────────────────────────────────────────
# Helper Functions
# ─────────────────────────────────────────────────────────────────────────────

print_header() {
  local title="$1"
  echo ""
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${NC}"
  echo -e "${CYAN}${BOLD}  ▶︎ ${title}${NC}"
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${NC}"
  echo ""
}

print_step() {
  echo -e "${CYAN}▶︎${NC} ${BOLD}$1${NC}"
}

print_substep() {
  echo -e "  ${DIM}»${NC} $1"
}

print_success() {
  echo -e "${GREEN}◆${NC} $1"
}

print_error() {
  echo -e "${RED}✗${NC} $1" >&2
}

print_warning() {
  echo -e "${YELLOW}◆${NC} $1"
}

print_separator() {
  echo -e "${DIM}───────────────────────────────────────────────────────────────${NC}"
}

print_cmd() {
  echo -e "  ${DIM}│${NC} ${DIM}$1${NC}"
}

# ─────────────────────────────────────────────────────────────────────────────
# Prerequisites
# ─────────────────────────────────────────────────────────────────────────────

check_certbot() {
  if ! command -v "${CERTBOT}" &> /dev/null; then
    print_error "certbot not found"
    print_substep "Install with: ${YELLOW}sudo dnf install certbot${NC}"
    exit 1
  fi
}

check_root() {
  if [[ $EUID -ne 0 ]]; then
    print_error "This script must be run as root (sudo)"
    exit 1
  fi
}

setup_webroot() {
  print_substep "Setting up ACME webroot at ${CYAN}${WEBROOT}${NC}"
  mkdir -p "${WEBROOT}/.well-known/acme-challenge"
  chown -R core:core "${WEBROOT}"
  chmod -R 755 "${WEBROOT}"
}

# ─────────────────────────────────────────────────────────────────────────────
# Certificate Operations
# ─────────────────────────────────────────────────────────────────────────────

is_cert_valid() {
  local cert_name="$1"
  local cert_file="/etc/letsencrypt/live/${cert_name}/fullchain.pem"
  if [[ ! -f "${cert_file}" ]]; then
    return 1
  fi
  if openssl x509 -checkend "${CERTBOT_RENEW_BEFORE_SECONDS}" -noout -in "${cert_file}" >/dev/null 2>&1; then
    return 0
  fi
  return 1
}

request_cert() {
  local cert_name="$1"
  shift
  local domains=("$@")

  print_step "Requesting certificate: ${MAGENTA}${cert_name}${NC}"
  print_substep "Domains: ${CYAN}${domains[*]}${NC}"

  # Build -d flags
  local domain_flags=()
  for d in "${domains[@]}"; do
    domain_flags+=("-d" "${d}")
  done

  # Build optional flags
  local extra_flags=()
  if [[ "${DRY_RUN}" == "true" ]]; then
    extra_flags+=("--dry-run")
    print_warning "DRY-RUN mode: no certificate will be issued"
  fi

  print_cmd "${CERTBOT} certonly --webroot -w ${WEBROOT} ${domain_flags[*]} --cert-name ${cert_name}"

  "${CERTBOT}" certonly \
    --non-interactive \
    --webroot \
    --webroot-path "${WEBROOT}" \
    --email "${EMAIL}" \
    --agree-tos \
    --no-eff-email \
    --keep-until-expiring \
    --expand \
    --cert-name "${cert_name}" \
    "${extra_flags[@]}" \
    "${domain_flags[@]}"

  # Skip copy in dry-run mode
  if [[ "${DRY_RUN}" != "true" ]]; then
    copy_cert "${cert_name}"
  fi
}

copy_cert() {
  local cert_name="$1"
  local cert_dir="${CERTS_DIR}/${cert_name}"
  local le_dir="/etc/letsencrypt/live/${cert_name}"

  print_substep "Copying certificate to ${CYAN}${cert_dir}/${NC}"

  mkdir -p "${cert_dir}"

  # Remove old files (symlinks or regular files)
  rm -f "${cert_dir}/fullchain.pem" "${cert_dir}/privkey.pem"

  # Copy certificates (dereference symlinks in /etc/letsencrypt/live/)
  cp -L "${le_dir}/fullchain.pem" "${cert_dir}/fullchain.pem"
  cp -L "${le_dir}/privkey.pem" "${cert_dir}/privkey.pem"

  # Fix ownership and permissions
  chown core:core "${cert_dir}/fullchain.pem" "${cert_dir}/privkey.pem"
  chmod 644 "${cert_dir}/fullchain.pem"
  chmod 600 "${cert_dir}/privkey.pem"

  print_success "${cert_name} copied"
}

request_all() {
  check_root
  check_certbot
  setup_webroot

  if [[ "${DRY_RUN}" == "true" ]]; then
    print_header "LET'S ENCRYPT (DRY-RUN)"
  else
    print_header "LET'S ENCRYPT"
  fi

  for cert_name in "${!DOMAINS[@]}"; do
    # Split domains string into array
    read -ra domain_array <<< "${DOMAINS[${cert_name}]}"
    if is_cert_valid "${cert_name}"; then
      print_success "${cert_name}: cert valid"
      if [[ "${DRY_RUN}" == "true" ]]; then
        print_warning "DRY-RUN mode: would copy existing certificate"
      else
        copy_cert "${cert_name}"
      fi
    else
      print_warning "${cert_name}: cert missing or expiring"
      request_cert "${cert_name}" "${domain_array[@]}"
    fi
    echo ""
  done

  # Skip hook installation in dry-run mode
  if [[ "${DRY_RUN}" != "true" ]]; then
    install_hook
  fi

  print_separator
  echo -e "${GREEN}${BOLD}◆ Certificate operations complete${NC}"
  echo ""

  if [[ "${DRY_RUN}" == "true" ]]; then
    print_substep "DRY-RUN complete. No certificates were issued."
    print_substep "Run without --dry-run to request real certificates."
  else
    print_substep "Certificates installed:"
    for cert_name in "${!DOMAINS[@]}"; do
      echo -e "    ${DIM}│${NC} ${CYAN}${CERTS_DIR}/${cert_name}/${NC}"
    done
    echo ""
    print_substep "Restart neonsignal to load new certificates:"
    echo -e "    ${DIM}│${NC} sudo systemctl restart neonsignal.service"
  fi
  echo ""
}

verify_certs() {
  print_header "CERTIFICATE STATUS"

  for cert_name in "${!DOMAINS[@]}"; do
    local cert_file="${CERTS_DIR}/${cert_name}/fullchain.pem"
    if [[ -f "${cert_file}" ]] || [[ -L "${cert_file}" ]]; then
      local subject expiry days_left
      subject=$(openssl x509 -in "${cert_file}" -noout -subject 2>/dev/null | sed 's/subject=//')
      expiry=$(openssl x509 -in "${cert_file}" -noout -enddate 2>/dev/null | cut -d= -f2)
      days_left=$(( ($(date -d "${expiry}" +%s) - $(date +%s)) / 86400 ))

      if [[ ${days_left} -lt 30 ]]; then
        print_warning "${cert_name}"
      else
        print_success "${cert_name}"
      fi
      echo -e "    ${DIM}│${NC} Subject: ${DIM}${subject}${NC}"
      echo -e "    ${DIM}│${NC} Expires: ${CYAN}${expiry}${NC} (${days_left} days)"
      echo ""
    else
      print_error "${cert_name}: NOT FOUND"
      echo ""
    fi
  done
}

renew_certs() {
  check_root
  check_certbot

  print_header "CERTIFICATE RENEWAL"

  print_step "Running certificate renewal"
  "${CERTBOT}" renew

  # Re-copy all certs (certbot may have renewed them)
  print_separator
  print_step "Syncing renewed certificates"
  for cert_name in "${!DOMAINS[@]}"; do
    if [[ -d "/etc/letsencrypt/live/${cert_name}" ]]; then
      copy_cert "${cert_name}"
    fi
  done

  print_separator
  print_step "Restarting neonsignal"
  systemctl restart neonsignal.service
  print_success "Done"
}

install_hook() {
  local hook_dir="/etc/letsencrypt/renewal-hooks/post"
  local hook_file="${hook_dir}/neonsignal-reload.sh"

  print_step "Installing renewal hook"

  mkdir -p "${hook_dir}"

  cat > "${hook_file}" << 'HOOK'
#!/usr/bin/env bash
# Post-renewal hook: copy renewed certs and restart neonsignal
set -euo pipefail

LOG="/var/log/neonsignal-renewal.log"
CERTS_DIR="/home/core/code/neonsignal/certs"

# Domains managed by neonsignal
DOMAINS=(
  "simonedelpopolo.com"
  "nutsloop.com"
  "neonsignal.nutsloop.com"
)

echo "$(date): Certificate renewal detected, copying certificates..." >> "${LOG}"

for domain in "${DOMAINS[@]}"; do
  le_dir="/etc/letsencrypt/live/${domain}"
  cert_dir="${CERTS_DIR}/${domain}"

  if [[ -d "${le_dir}" ]]; then
    mkdir -p "${cert_dir}"
    cp -L "${le_dir}/fullchain.pem" "${cert_dir}/fullchain.pem"
    cp -L "${le_dir}/privkey.pem" "${cert_dir}/privkey.pem"
    chown core:core "${cert_dir}/fullchain.pem" "${cert_dir}/privkey.pem"
    chmod 644 "${cert_dir}/fullchain.pem"
    chmod 600 "${cert_dir}/privkey.pem"
    echo "$(date): Copied ${domain}" >> "${LOG}"
  fi
done

echo "$(date): Restarting neonsignal..." >> "${LOG}"
systemctl restart neonsignal.service
echo "$(date): neonsignal restarted successfully" >> "${LOG}"
HOOK

  chmod +x "${hook_file}"
  print_success "Renewal hook installed at ${DIM}${hook_file}${NC}"
}

# ─────────────────────────────────────────────────────────────────────────────
# Usage
# ─────────────────────────────────────────────────────────────────────────────

show_usage() {
  print_header "LET'S ENCRYPT USAGE"

  echo -e "${BOLD}Commands:${NC}"
  echo -e "  ${CYAN}request-all${NC}                        Request all configured certificates"
  echo -e "  ${CYAN}request${NC} <name> <domain1> [dom2..]  Request a single certificate"
  echo -e "  ${CYAN}copy${NC} <name>                        Copy existing LE cert to certs/<name>/"
  echo -e "  ${CYAN}verify${NC}                             Show certificate status"
  echo -e "  ${CYAN}renew${NC}                              Renew all certificates"
  echo -e "  ${CYAN}install-hook${NC}                       Install post-renewal hook"
  echo -e "  ${CYAN}help${NC}                               Show this help"
  echo ""
  echo -e "${BOLD}Options:${NC}"
  echo -e "  ${YELLOW}--dry-run${NC}                          Test without issuing real certificates"
  echo ""
  print_separator
  echo -e "${BOLD}Configured domains:${NC}"
  for cert_name in "${!DOMAINS[@]}"; do
    echo -e "  ${DIM}│${NC} ${MAGENTA}${cert_name}${NC}: ${DIM}${DOMAINS[${cert_name}]}${NC}"
  done
  echo ""
  echo -e "${BOLD}Environment:${NC}"
  echo -e "  ${DIM}│${NC} EMAIL=${CYAN}${EMAIL}${NC}"
  echo -e "  ${DIM}│${NC} WEBROOT=${CYAN}${WEBROOT}${NC}"
  echo -e "  ${DIM}│${NC} CERTS_DIR=${CYAN}${CERTS_DIR}${NC}"
  echo ""
}

# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────

main() {
  # Parse global flags
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --dry-run)
        DRY_RUN="true"
        shift
        ;;
      *)
        break
        ;;
    esac
  done

  local cmd="${1:-help}"

  case "${cmd}" in
    request-all)
      request_all
      ;;

    request)
      check_root
      check_certbot
      setup_webroot
      shift
      if [[ $# -lt 2 ]]; then
        print_error "Usage: $0 request <cert-name> <domain1> [domain2...]"
        exit 1
      fi
      request_cert "$@"
      ;;

    copy)
      check_root
      shift
      if [[ $# -ne 1 ]]; then
        print_error "Usage: $0 copy <cert-name>"
        exit 1
      fi
      copy_cert "$1"
      ;;

    verify)
      verify_certs
      ;;

    renew)
      renew_certs
      ;;

    install-hook)
      check_root
      install_hook
      ;;

    help|--help|-h)
      show_usage
      ;;

    *)
      print_error "Unknown command: ${cmd}"
      show_usage
      exit 1
      ;;
  esac
}

main "$@"
