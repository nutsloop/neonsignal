#!/usr/bin/env bash
set -euo pipefail

# ═══════════════════════════════════════════════════════════════════════════════
# NeonSignal Certificate Issuer (Local CA)
# ═══════════════════════════════════════════════════════════════════════════════
#
# Local development certificate management using a self-signed CA.
#
# Usage:
#   ./scripts/certificates/issuer.sh --local generate-all
#   ./scripts/certificates/issuer.sh --local generate <hostname> [dir_name]
#   ./scripts/certificates/issuer.sh --local verify
#   ./scripts/certificates/issuer.sh status
#   ./scripts/certificates/issuer.sh help

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

# ─────────────────────────────────────────────────────────────────────────────
# Configuration
# ─────────────────────────────────────────────────────────────────────────────

# Local development hosts (format: "hostname" or "hostname:dir_name")
LOCAL_HOSTS=(
  "10.0.0.106:_default"
)

# ─────────────────────────────────────────────────────────────────────────────
# Shared Helpers
# ─────────────────────────────────────────────────────────────────────────────

print_separator() {
  echo -e "${DIM}───────────────────────────────────────────────────────────────${RESET}"
}

# ─────────────────────────────────────────────────────────────────────────────
# Local CA Functions
# ─────────────────────────────────────────────────────────────────────────────

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
  cat > "${leaf_cnf}" <<LEAFEOF
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
LEAFEOF

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

# ─────────────────────────────────────────────────────────────────────────────
# Status
# ─────────────────────────────────────────────────────────────────────────────

show_status() {
  print_header "NeonSignal Certificate Status"
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

# ─────────────────────────────────────────────────────────────────────────────
# Help
# ─────────────────────────────────────────────────────────────────────────────

show_help() {
  print_header "NeonSignal Certificate Issuer"

  echo -e "${BOLD}Usage:${RESET}"
  echo -e "  ${CYAN}issuer.sh${RESET} ${YELLOW}--local${RESET} <command>"
  echo -e "  ${CYAN}issuer.sh${RESET} status"
  echo -e "  ${CYAN}issuer.sh${RESET} help"
  echo ""

  echo -e "${BOLD}Local CA Commands:${RESET}"
  echo -e "  ${CYAN}generate-all${RESET}                       Generate all configured certificates"
  echo -e "  ${CYAN}generate${RESET} <hostname> [dir_name]     Generate a single certificate"
  echo -e "  ${CYAN}verify${RESET}                             Show certificate status"
  echo ""

  print_separator
  echo -e "${BOLD}Configured Local Hosts:${RESET}"
  for entry in "${LOCAL_HOSTS[@]}"; do
    echo -e "  ${DIM}│${RESET} ${MAGENTA}${entry}${RESET}"
  done
  echo ""

  echo -e "${BOLD}Environment:${RESET}"
  echo -e "  ${DIM}│${RESET} CERTS_DIR=${CYAN}${NEONSIGNAL_CERTS_DIR}${RESET}"
  echo -e "  ${DIM}│${RESET} CA_DIR=${CYAN}${NEONSIGNAL_CERTS_CA_DIR}${RESET}"
  echo ""
}

# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────

main() {
  # Handle no arguments
  if [[ $# -eq 0 ]]; then
    show_help
    exit 0
  fi

  # Parse mode flag
  case "${1:-}" in
    --local)
      shift
      handle_local "$@"
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
}

main "$@"
