#!/usr/bin/env bash
set -euo pipefail

# Virtual hosts to generate certificates for
# Format: "hostname:directory_name"
# The directory_name is optional; defaults to hostname
VHOSTS=(
  "simonedelpopolo.host"
  "neonsignal.nutsloop.host"
  "10.0.0.10:_default"
  "nutsloop.host"
)

OUT_DIR="${OUT_DIR:-certs}"
CA_DIR="${OUT_DIR}/ca"

mkdir -p "${CA_DIR}"

# 1) Create local CA if missing.
if [[ ! -f "${CA_DIR}/root.crt" || ! -f "${CA_DIR}/root.key" ]]; then
  echo "Generating local CA..."
  openssl genrsa -out "${CA_DIR}/root.key" 4096
  openssl req -x509 -new -nodes -key "${CA_DIR}/root.key" -sha256 -days 3650 \
    -out "${CA_DIR}/root.crt" -subj "/CN=neonsignal-local-CA"
fi

# 2) Generate certificate for each virtual host
generate_cert() {
  local host="$1"
  local dir_name="$2"
  local cert_dir="${OUT_DIR}/${dir_name}"
  local leaf_cnf="${cert_dir}/leaf.cnf"
  local key="${cert_dir}/privkey.pem"
  local crt="${cert_dir}/fullchain.pem"
  local csr="${cert_dir}/server.csr"

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

  echo "Generating certificate for ${host} -> ${cert_dir}..."
  openssl genrsa -out "${key}" 2048 2>/dev/null
  openssl req -new -key "${key}" -out "${csr}" -config "${leaf_cnf}"

  openssl x509 -req -in "${csr}" \
    -CA "${CA_DIR}/root.crt" -CAkey "${CA_DIR}/root.key" -CAcreateserial \
    -out "${crt}" -days 825 -sha256 -extfile "${leaf_cnf}" -extensions req_ext 2>/dev/null

  # Clean up intermediate files
  rm -f "${csr}" "${leaf_cnf}"

  echo "  ✓ ${crt}"
  echo "  ✓ ${key}"
}

echo ""
echo "=== Generating virtual host certificates ==="
echo ""

for entry in "${VHOSTS[@]}"; do
  # Parse "host:dir" or just "host"
  if [[ "${entry}" == *":"* ]]; then
    host="${entry%%:*}"
    dir_name="${entry##*:}"
  else
    host="${entry}"
    dir_name="${entry}"
  fi

  generate_cert "${host}" "${dir_name}"
  echo ""
done

echo "=== Done ==="
echo ""
echo "Certificate directories created:"
for entry in "${VHOSTS[@]}"; do
  if [[ "${entry}" == *":"* ]]; then
    dir_name="${entry##*:}"
  else
    dir_name="${entry}"
  fi
  echo "  ${OUT_DIR}/${dir_name}/"
done
echo ""
echo "CA certificate (trust this in your OS):"
echo "  ${CA_DIR}/root.crt"
echo ""
