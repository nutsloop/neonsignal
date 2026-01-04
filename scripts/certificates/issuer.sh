#!/usr/bin/env bash
set -euo pipefail

# ═══════════════════════════════════════════════════════════════════════════════
# NeonSignal Certificate Issuer
# ═══════════════════════════════════════════════════════════════════════════════
#
# Unified certificate management for production (Let's Encrypt) and local dev.
#
# Usage:
#   # Production certificates (Let's Encrypt) - requires sudo
#   sudo ./scripts/certificates/issuer.sh --letsencrypt request-all
#   sudo ./scripts/certificates/issuer.sh --letsencrypt --dry-run request-all
#   sudo ./scripts/certificates/issuer.sh --letsencrypt request <name> <domain1> [domain2...]
#   sudo ./scripts/certificates/issuer.sh --letsencrypt verify
#   sudo ./scripts/certificates/issuer.sh --letsencrypt renew
#   sudo ./scripts/certificates/issuer.sh --letsencrypt install-hook
#
#   # Local development certificates (self-signed CA)
#   ./scripts/certificates/issuer.sh --local generate-all
#   ./scripts/certificates/issuer.sh --local generate <hostname> [dir_name]
#   ./scripts/certificates/issuer.sh --local verify
#
#   # Shared commands
#   ./scripts/certificates/issuer.sh status
#   ./scripts/certificates/issuer.sh help

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

# ─────────────────────────────────────────────────────────────────────────────────
# Configuration
# ─────────────────────────────────────────────────────────────────────────────────

# Production domains (Let's Encrypt)
declare -A LE_DOMAINS=(
  ["simonedelpopolo.com"]="simonedelpopolo.com www.simonedelpopolo.com"
  ["nutsloop.com"]="nutsloop.com www.nutsloop.com"
  ["neonsignal.nutsloop.com"]="neonsignal.nutsloop.com"
)

# Local development hosts (format: "hostname" or "hostname:dir_name")
LOCAL_HOSTS=(
  "simonedelpopolo.host"
  "neonsignal.nutsloop.host"
  "10.0.0.10:_default"
  "nutsloop.host"
)

# Runtime state
DRY_RUN="false"
MODE=""
CERTBOT="${CERTBOT:-certbot}"
CERTBOT_RENEW_BEFORE_SECONDS="${CERTBOT_RENEW_BEFORE_SECONDS:-2592000}" # 30 days

# ─────────────────────────────────────────────────────────────────────────────────
# Shared Helpers
# ─────────────────────────────────────────────────────────────────────────────────

print_separator() {
  echo -e "${DIM}───────────────────────────────────────────────────────────────${RESET}"
}

print_cmd() {
  echo -e "  ${DIM}│${RESET} ${DIM}$1${RESET}"
}

# ─────────────────────────────────────────────────────────────────────────────────
# Let's Encrypt Functions
# ─────────────────────────────────────────────────────────────────────────────────

le_check_root() {
  if [[ $EUID -ne 0 ]]; then
    print_error "Let's Encrypt mode requires root (sudo)"
    exit 1
  fi
}

le_check_certbot() {
  if ! command -v "${CERTBOT}" &> /dev/null; then
    print_error "certbot not found"
    print_substep "Install with: sudo dnf install certbot"
    exit 1
  fi
}

le_setup_webroot() {
  print_substep "Setting up ACME webroot at ${CYAN}${NEONSIGNAL_ACME_WEBROOT}${RESET}"
  mkdir -p "${NEONSIGNAL_ACME_WEBROOT}/.well-known/acme-challenge"
  chown -R core:core "${NEONSIGNAL_ACME_WEBROOT}"
  chmod -R 755 "${NEONSIGNAL_ACME_WEBROOT}"
}

le_is_cert_valid() {
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

le_request_cert() {
  local cert_name="$1"
  shift
  local domains=("$@")

  print_step "Requesting certificate: ${MAGENTA}${cert_name}${RESET}"
  print_substep "Domains: ${CYAN}${domains[*]}${RESET}"

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

  print_cmd "${CERTBOT} certonly --webroot -w ${NEONSIGNAL_ACME_WEBROOT} ${domain_flags[*]} --cert-name ${cert_name}"

  "${CERTBOT}" certonly \
    --non-interactive \
    --webroot \
    --webroot-path "${NEONSIGNAL_ACME_WEBROOT}" \
    --email "${NEONSIGNAL_LETSENCRYPT_EMAIL}" \
    --agree-tos \
    --no-eff-email \
    --keep-until-expiring \
    --expand \
    --cert-name "${cert_name}" \
    "${extra_flags[@]}" \
    "${domain_flags[@]}"

  # Skip copy in dry-run mode
  if [[ "${DRY_RUN}" != "true" ]]; then
    le_copy_cert "${cert_name}"
  fi
}

le_copy_cert() {
  local cert_name="$1"
  local cert_dir="${NEONSIGNAL_CERTS_DIR}/${cert_name}"
  local le_dir="/etc/letsencrypt/live/${cert_name}"

  print_substep "Copying certificate to ${CYAN}${cert_dir}/${RESET}"

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

le_request_all() {
  le_check_root
  le_check_certbot
  le_setup_webroot

  if [[ "${DRY_RUN}" == "true" ]]; then
    print_header "NeonSignal Certificate Issuer (LE DRY-RUN)"
  else
    print_header "NeonSignal Certificate Issuer (Let's Encrypt)"
  fi

  for cert_name in "${!LE_DOMAINS[@]}"; do
    # Split domains string into array
    read -ra domain_array <<< "${LE_DOMAINS[${cert_name}]}"
    if le_is_cert_valid "${cert_name}"; then
      print_success "${cert_name}: cert valid"
      if [[ "${DRY_RUN}" == "true" ]]; then
        print_warning "DRY-RUN mode: would copy existing certificate"
      else
        le_copy_cert "${cert_name}"
      fi
    else
      print_warning "${cert_name}: cert missing or expiring"
      le_request_cert "${cert_name}" "${domain_array[@]}"
    fi
    echo ""
  done

  # Skip hook installation in dry-run mode
  if [[ "${DRY_RUN}" != "true" ]]; then
    le_install_hook
  fi

  print_separator
  echo -e "${GREEN}${BOLD}Certificate operations complete${RESET}"
  echo ""

  if [[ "${DRY_RUN}" == "true" ]]; then
    print_substep "DRY-RUN complete. No certificates were issued."
    print_substep "Run without --dry-run to request real certificates."
  else
    print_substep "Certificates installed:"
    for cert_name in "${!LE_DOMAINS[@]}"; do
      echo -e "    ${DIM}│${RESET} ${CYAN}${NEONSIGNAL_CERTS_DIR}/${cert_name}/${RESET}"
    done
    echo ""
    print_substep "Restart neonsignal to load new certificates:"
    echo -e "    ${DIM}│${RESET} sudo systemctl restart neonsignal.service"
  fi
  echo ""
}

le_renew() {
  le_check_root
  le_check_certbot

  print_header "NeonSignal Certificate Renewal"

  print_step "Running certificate renewal"
  "${CERTBOT}" renew

  # Re-copy all certs (certbot may have renewed them)
  print_separator
  print_step "Syncing renewed certificates"
  for cert_name in "${!LE_DOMAINS[@]}"; do
    if [[ -d "/etc/letsencrypt/live/${cert_name}" ]]; then
      le_copy_cert "${cert_name}"
    fi
  done

  print_separator
  print_step "Restarting neonsignal"
  systemctl restart neonsignal.service
  print_success "Done"
}

le_install_hook() {
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
  print_success "Renewal hook installed at ${DIM}${hook_file}${RESET}"
}

le_verify() {
  print_header "Let's Encrypt Certificate Status"

  for cert_name in "${!LE_DOMAINS[@]}"; do
    local cert_file="${NEONSIGNAL_CERTS_DIR}/${cert_name}/fullchain.pem"
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
      echo -e "    ${DIM}│${RESET} Subject: ${DIM}${subject}${RESET}"
      echo -e "    ${DIM}│${RESET} Expires: ${CYAN}${expiry}${RESET} (${days_left} days)"
      echo ""
    else
      print_error "${cert_name}: NOT FOUND"
      echo ""
    fi
  done
}

handle_letsencrypt() {
  local cmd="${1:-help}"
  shift || true

  case "${cmd}" in
    request-all)
      le_request_all
      ;;
    request)
      le_check_root
      le_check_certbot
      le_setup_webroot
      if [[ $# -lt 2 ]]; then
        print_error "Usage: issuer.sh --letsencrypt request <cert-name> <domain1> [domain2...]"
        exit 1
      fi
      le_request_cert "$@"
      ;;
    copy)
      le_check_root
      if [[ $# -ne 1 ]]; then
        print_error "Usage: issuer.sh --letsencrypt copy <cert-name>"
        exit 1
      fi
      le_copy_cert "$1"
      ;;
    verify)
      le_verify
      ;;
    renew)
      le_renew
      ;;
    install-hook)
      le_check_root
      le_install_hook
      ;;
    help|*)
      show_help
      ;;
  esac
}

# ─────────────────────────────────────────────────────────────────────────────────
# Local CA Functions
# ─────────────────────────────────────────────────────────────────────────────────

local_ensure_ca() {
  if [[ ! -f "${NEONSIGNAL_CERTS_CA_DIR}/root.crt" || ! -f "${NEONSIGNAL_CERTS_CA_DIR}/root.key" ]]; then
    print_step "Generating local CA"
    mkdir -p "${NEONSIGNAL_CERTS_CA_DIR}"
    openssl genrsa -out "${NEONSIGNAL_CERTS_CA_DIR}/root.key" 4096 2>/dev/null
    openssl req -x509 -new -nodes -key "${NEONSIGNAL_CERTS_CA_DIR}/root.key" -sha256 -days 3650 \
      -out "${NEONSIGNAL_CERTS_CA_DIR}/root.crt" -subj "/CN=neonsignal-local-CA"
    print_success "Local CA created at ${CYAN}${NEONSIGNAL_CERTS_CA_DIR}/${RESET}"
  else
    print_substep "Local CA exists at ${CYAN}${NEONSIGNAL_CERTS_CA_DIR}/${RESET}"
  fi
}

local_generate_cert() {
  local host="$1"
  local dir_name="${2:-$host}"
  local cert_dir="${NEONSIGNAL_CERTS_DIR}/${dir_name}"
  local leaf_cnf="${cert_dir}/leaf.cnf"
  local key="${cert_dir}/privkey.pem"
  local crt="${cert_dir}/fullchain.pem"
  local csr="${cert_dir}/server.csr"

  print_step "Generating certificate for ${MAGENTA}${host}${RESET}"
  print_substep "Output: ${CYAN}${cert_dir}/${RESET}"

  mkdir -p "${cert_dir}"

  # Leaf config with SAN
  cat > "${leaf_cnf}" <<EOF
[req]
default_bits = 2048
prompt = no
default_md = sha256
req_extensions = req_ext
distinguished_name = dn

[dn]
CN = ${host}

[req_ext]
subjectAltName = @alt_names

[alt_names]
EOF

  # Add appropriate SAN entry (IP or DNS)
  if [[ "${host}" =~ ^([0-9]{1,3}\.){3}[0-9]{1,3}$ ]]; then
    echo "IP.1 = ${host}" >> "${leaf_cnf}"
  else
    echo "DNS.1 = ${host}" >> "${leaf_cnf}"
  fi

  openssl genrsa -out "${key}" 2048 2>/dev/null
  openssl req -new -key "${key}" -out "${csr}" -config "${leaf_cnf}"

  openssl x509 -req -in "${csr}" \
    -CA "${NEONSIGNAL_CERTS_CA_DIR}/root.crt" -CAkey "${NEONSIGNAL_CERTS_CA_DIR}/root.key" -CAcreateserial \
    -out "${crt}" -days 825 -sha256 -extfile "${leaf_cnf}" -extensions req_ext 2>/dev/null

  # Clean up intermediate files
  rm -f "${csr}" "${leaf_cnf}"

  print_success "Created ${CYAN}${crt}${RESET}"
  print_success "Created ${CYAN}${key}${RESET}"
}

local_generate_all() {
  print_header "NeonSignal Certificate Issuer (Local CA)"

  local_ensure_ca
  echo ""

  for entry in "${LOCAL_HOSTS[@]}"; do
    # Parse "host:dir" or just "host"
    if [[ "${entry}" == *":"* ]]; then
      host="${entry%%:*}"
      dir_name="${entry##*:}"
    else
      host="${entry}"
      dir_name="${entry}"
    fi

    local_generate_cert "${host}" "${dir_name}"
    echo ""
  done

  print_separator
  echo -e "${GREEN}${BOLD}Local certificates generated${RESET}"
  echo ""
  print_substep "Certificate directories:"
  for entry in "${LOCAL_HOSTS[@]}"; do
    if [[ "${entry}" == *":"* ]]; then
      dir_name="${entry##*:}"
    else
      dir_name="${entry}"
    fi
    echo -e "    ${DIM}│${RESET} ${CYAN}${NEONSIGNAL_CERTS_DIR}/${dir_name}/${RESET}"
  done
  echo ""
  print_substep "CA certificate (trust in your OS):"
  echo -e "    ${DIM}│${RESET} ${CYAN}${NEONSIGNAL_CERTS_CA_DIR}/root.crt${RESET}"
  echo ""
}

local_verify() {
  print_header "Local Certificate Status"

  for entry in "${LOCAL_HOSTS[@]}"; do
    if [[ "${entry}" == *":"* ]]; then
      dir_name="${entry##*:}"
      host="${entry%%:*}"
    else
      dir_name="${entry}"
      host="${entry}"
    fi

    local cert_file="${NEONSIGNAL_CERTS_DIR}/${dir_name}/fullchain.pem"
    if [[ -f "${cert_file}" ]]; then
      local subject expiry days_left
      subject=$(openssl x509 -in "${cert_file}" -noout -subject 2>/dev/null | sed 's/subject=//')
      expiry=$(openssl x509 -in "${cert_file}" -noout -enddate 2>/dev/null | cut -d= -f2)
      days_left=$(( ($(date -d "${expiry}" +%s) - $(date +%s)) / 86400 ))

      if [[ ${days_left} -lt 30 ]]; then
        print_warning "${dir_name} (${host})"
      else
        print_success "${dir_name} (${host})"
      fi
      echo -e "    ${DIM}│${RESET} Subject: ${DIM}${subject}${RESET}"
      echo -e "    ${DIM}│${RESET} Expires: ${CYAN}${expiry}${RESET} (${days_left} days)"
      echo ""
    else
      print_error "${dir_name}: NOT FOUND"
      echo ""
    fi
  done
}

handle_local() {
  local cmd="${1:-help}"
  shift || true

  case "${cmd}" in
    generate-all)
      local_generate_all
      ;;
    generate)
      print_header "NeonSignal Certificate Issuer (Local CA)"
      local_ensure_ca
      echo ""
      if [[ $# -lt 1 ]]; then
        print_error "Usage: issuer.sh --local generate <hostname> [dir_name]"
        exit 1
      fi
      local_generate_cert "$@"
      ;;
    verify)
      local_verify
      ;;
    help|*)
      show_help
      ;;
  esac
}

# ─────────────────────────────────────────────────────────────────────────────────
# Status (All Certificates)
# ─────────────────────────────────────────────────────────────────────────────────

show_status() {
  print_header "NeonSignal Certificate Status"

  print_step "Let's Encrypt Certificates"
  echo ""
  for cert_name in "${!LE_DOMAINS[@]}"; do
    local cert_file="${NEONSIGNAL_CERTS_DIR}/${cert_name}/fullchain.pem"
    if [[ -f "${cert_file}" ]]; then
      local expiry days_left
      expiry=$(openssl x509 -in "${cert_file}" -noout -enddate 2>/dev/null | cut -d= -f2)
      days_left=$(( ($(date -d "${expiry}" +%s) - $(date +%s)) / 86400 ))
      if [[ ${days_left} -lt 30 ]]; then
        print_warning "${cert_name}: ${days_left} days"
      else
        print_success "${cert_name}: ${days_left} days"
      fi
    else
      print_error "${cert_name}: NOT FOUND"
    fi
  done

  echo ""
  print_step "Local Development Certificates"
  echo ""
  for entry in "${LOCAL_HOSTS[@]}"; do
    if [[ "${entry}" == *":"* ]]; then
      dir_name="${entry##*:}"
    else
      dir_name="${entry}"
    fi
    local cert_file="${NEONSIGNAL_CERTS_DIR}/${dir_name}/fullchain.pem"
    if [[ -f "${cert_file}" ]]; then
      local expiry days_left
      expiry=$(openssl x509 -in "${cert_file}" -noout -enddate 2>/dev/null | cut -d= -f2)
      days_left=$(( ($(date -d "${expiry}" +%s) - $(date +%s)) / 86400 ))
      if [[ ${days_left} -lt 30 ]]; then
        print_warning "${dir_name}: ${days_left} days"
      else
        print_success "${dir_name}: ${days_left} days"
      fi
    else
      print_error "${dir_name}: NOT FOUND"
    fi
  done
  echo ""
}

# ─────────────────────────────────────────────────────────────────────────────────
# Help
# ─────────────────────────────────────────────────────────────────────────────────

show_help() {
  print_header "NeonSignal Certificate Issuer"

  echo -e "${BOLD}Usage:${RESET}"
  echo -e "  ${CYAN}issuer.sh${RESET} ${YELLOW}--letsencrypt${RESET} [--dry-run] <command>"
  echo -e "  ${CYAN}issuer.sh${RESET} ${YELLOW}--local${RESET} <command>"
  echo -e "  ${CYAN}issuer.sh${RESET} status"
  echo -e "  ${CYAN}issuer.sh${RESET} help"
  echo ""

  echo -e "${BOLD}Let's Encrypt Commands:${RESET} ${DIM}(requires sudo)${RESET}"
  echo -e "  ${CYAN}request-all${RESET}                        Request all configured certificates"
  echo -e "  ${CYAN}request${RESET} <name> <domain1> [dom2..]  Request a single certificate"
  echo -e "  ${CYAN}copy${RESET} <name>                        Copy existing LE cert to certs/<name>/"
  echo -e "  ${CYAN}verify${RESET}                             Show certificate status"
  echo -e "  ${CYAN}renew${RESET}                              Renew all certificates"
  echo -e "  ${CYAN}install-hook${RESET}                       Install post-renewal hook"
  echo ""

  echo -e "${BOLD}Local CA Commands:${RESET}"
  echo -e "  ${CYAN}generate-all${RESET}                       Generate all configured certificates"
  echo -e "  ${CYAN}generate${RESET} <hostname> [dir_name]     Generate a single certificate"
  echo -e "  ${CYAN}verify${RESET}                             Show certificate status"
  echo ""

  echo -e "${BOLD}Options:${RESET}"
  echo -e "  ${YELLOW}--dry-run${RESET}                          Test LE without issuing real certs"
  echo ""

  print_separator
  echo -e "${BOLD}Configured Let's Encrypt Domains:${RESET}"
  for cert_name in "${!LE_DOMAINS[@]}"; do
    echo -e "  ${DIM}│${RESET} ${MAGENTA}${cert_name}${RESET}: ${DIM}${LE_DOMAINS[${cert_name}]}${RESET}"
  done
  echo ""

  echo -e "${BOLD}Configured Local Hosts:${RESET}"
  for entry in "${LOCAL_HOSTS[@]}"; do
    echo -e "  ${DIM}│${RESET} ${MAGENTA}${entry}${RESET}"
  done
  echo ""

  echo -e "${BOLD}Environment:${RESET}"
  echo -e "  ${DIM}│${RESET} EMAIL=${CYAN}${NEONSIGNAL_LETSENCRYPT_EMAIL}${RESET}"
  echo -e "  ${DIM}│${RESET} WEBROOT=${CYAN}${NEONSIGNAL_ACME_WEBROOT}${RESET}"
  echo -e "  ${DIM}│${RESET} CERTS_DIR=${CYAN}${NEONSIGNAL_CERTS_DIR}${RESET}"
  echo -e "  ${DIM}│${RESET} CA_DIR=${CYAN}${NEONSIGNAL_CERTS_CA_DIR}${RESET}"
  echo ""
}

# ─────────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────────

main() {
  # Handle no arguments
  if [[ $# -eq 0 ]]; then
    show_help
    exit 0
  fi

  # Parse mode flag
  case "${1:-}" in
    --letsencrypt)
      MODE="letsencrypt"
      shift
      ;;
    --local)
      MODE="local"
      shift
      ;;
    status)
      show_status
      exit 0
      ;;
    help|--help|-h)
      show_help
      exit 0
      ;;
    *)
      print_error "Unknown mode: ${1}"
      echo ""
      show_help
      exit 1
      ;;
  esac

  # Parse --dry-run if present
  if [[ "${1:-}" == "--dry-run" ]]; then
    DRY_RUN="true"
    shift
  fi

  # Dispatch to mode handler
  case "$MODE" in
    letsencrypt)
      handle_letsencrypt "$@"
      ;;
    local)
      handle_local "$@"
      ;;
  esac
}

main "$@"
