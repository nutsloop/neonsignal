#!/usr/bin/env bash
# Multi-endpoint h2load benchmark (max concurrency)
# Targets HTTPS vhosts + HTTP redirect, static assets, 404s, and auth-required APIs.

set -euo pipefail

CONNECT_HOST="${CONNECT_HOST:-10.0.0.10}"
HTTPS_PORT="${HTTPS_PORT:-9443}"
HTTP_PORT="${HTTP_PORT:-9090}"

MAX_CLIENTS="${H2LOAD_MAX_CLIENTS:-1000}"
MAX_STREAMS="${H2LOAD_MAX_STREAMS:-100}"
THREADS="${H2LOAD_THREADS:-$(nproc)}"
DURATION="${H2LOAD_DURATION:-10}"
REQUESTS="${H2LOAD_REQUESTS:-50000}"

TS="${TS:-$(date -u +"%Y-%m-%d-%H%M%S")}"
OUT_DIR="${OUT_DIR:-/tmp/neonsignal-bench-${TS}}"
KEEP_RAW="${KEEP_RAW:-0}"

mkdir -p "$OUT_DIR"

run_h2load() {
  local label="$1"
  local url="$2"
  shift 2

  local out_file="${OUT_DIR}/${label}.txt"

  echo ""
  echo "==> ${label}"
  echo "    ${url}"

  h2load \
    -n "$REQUESTS" \
    -c "$MAX_CLIENTS" \
    -t "$THREADS" \
    -m "$MAX_STREAMS" \
    -D "$DURATION" \
    "$@" \
    "$url" \
    >"$out_file" 2>&1
}

summarize_file() {
  local label="$1"
  local file="$2"

  local reqs
  local req_s
  local latency

  reqs=$(awk '/requests:/{print; exit}' "$file")
  req_s=$(awk '/req\/s/{print $2; exit}' "$file")
  latency=$(awk -F':' '/time for request/{print $2; exit}' "$file")

  echo "-- ${label}"
  echo "   ${reqs:-requests: (missing)}"
  echo "   req/s: ${req_s:-?}"
  echo "   time for request:${latency:- ?}"
}

run_https_test() {
  local label="$1"
  local domain="$2"
  local path="$3"
  shift 3

  local url="https://${domain}:${HTTPS_PORT}${path}"

  run_h2load "$label" "$url" \
    --connect-to "${CONNECT_HOST}:${HTTPS_PORT}" \
    --sni "$domain" \
    "$@"
}

run_http_test() {
  local label="$1"
  local domain="$2"
  local path="$3"
  shift 3

  local url="http://${domain}:${HTTP_PORT}${path}"

  run_h2load "$label" "$url" \
    --h1 \
    --connect-to "${CONNECT_HOST}:${HTTP_PORT}" \
    "$@"
}

# Asset paths per vhost
simonedelpopolo_css="/css/wasteland.css"
simonedelpopolo_js="/scripts/visual-performance.js"

nutsloop_css="/css/style.css"
nutsloop_js="/app.js"

neonsignal_css="/css/app.css"
neonsignal_js="/app.js"

# Random 404 paths
rand_suffix="$(date +%s)-$RANDOM"

# Auth API headers/payload
auth_cookie_header="cookie: ns_session=invalid"
auth_user_header="x-user: bench@example.com"

payload_file="${OUT_DIR}/auth-payload.json"
cat <<'JSON' > "$payload_file"
{"dummy":true}
JSON

# HTTPS tests per domain
run_https_test "simonedelpopolo_root" "simonedelpopolo.host" "/"
run_https_test "simonedelpopolo_css" "simonedelpopolo.host" "$simonedelpopolo_css"
run_https_test "simonedelpopolo_js" "simonedelpopolo.host" "$simonedelpopolo_js"
run_https_test "simonedelpopolo_404" "simonedelpopolo.host" "/__bench_404_${rand_suffix}"

run_https_test "nutsloop_root" "nutsloop.host" "/"
run_https_test "nutsloop_css" "nutsloop.host" "$nutsloop_css"
run_https_test "nutsloop_js" "nutsloop.host" "$nutsloop_js"
run_https_test "nutsloop_404" "nutsloop.host" "/__bench_404_${rand_suffix}"

run_https_test "neonsignal_root" "neonsignal.nutsloop.host" "/"
run_https_test "neonsignal_css" "neonsignal.nutsloop.host" "$neonsignal_css"
run_https_test "neonsignal_js" "neonsignal.nutsloop.host" "$neonsignal_js"
run_https_test "neonsignal_404" "neonsignal.nutsloop.host" "/__bench_404_${rand_suffix}"

# Auth-related API checks (neonsignal host)
run_https_test "auth_login_options" "neonsignal.nutsloop.host" "/api/auth/login/options"
run_https_test "auth_user_check" "neonsignal.nutsloop.host" "/api/auth/user/check" \
  -H "$auth_user_header"
run_https_test "auth_user_enroll_get" "neonsignal.nutsloop.host" "/api/auth/user/enroll" \
  -H "$auth_cookie_header"
run_https_test "auth_user_enroll_post" "neonsignal.nutsloop.host" "/api/auth/user/enroll" \
  -H "$auth_cookie_header" \
  -H "content-type: application/json" \
  -d "$payload_file"

# HTTP redirect checks (port 9090)
run_http_test "redirect_root" "simonedelpopolo.host" "/"
run_http_test "redirect_404" "simonedelpopolo.host" "/__bench_404_${rand_suffix}"

# Summaries
for file in "$OUT_DIR"/*.txt; do
  label=$(basename "$file" .txt)
  summarize_file "$label" "$file"
  echo ""
done

if [ "$KEEP_RAW" -eq 0 ]; then
  rm -f "$OUT_DIR"/*.txt "$payload_file"
fi
