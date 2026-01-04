#!/usr/bin/env bash
# NeonSignal HTTP/2 Server Benchmark Suite
# Tests StaticFileCache, ConnectionManager, and throughput

set -euo pipefail

HOST="https://10.0.0.10:9443"
DURATION=10
WARMUP=2

CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
DIM='\033[2m'
BOLD='\033[1m'
RESET='\033[0m'

print_header() {
  echo ""
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo -e "${CYAN}${BOLD}  ▶︎ NeonSignal HTTP/2 Benchmark Suite${RESET}"
  echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${RESET}"
  echo ""
}

print_step() {
  echo -e "${CYAN}▶︎${RESET} ${BOLD}$1${RESET}"
}

print_substep() {
  echo -e "  ${DIM}»${RESET} $1"
}

print_success() {
  echo -e "${GREEN}◆${RESET} $1"
}

print_error() {
  echo -e "${RED}✗${RESET} $1" >&2
}

print_separator() {
  echo -e "${DIM}───────────────────────────────────────────────────────────────${RESET}"
}

print_header

# Check if server is running
if ! curl -sk "$HOST" > /dev/null 2>&1; then
  print_error "Server not reachable at ${HOST}"
  print_substep "Start the server first: sudo systemctl start neonsignal"
  exit 1
fi

print_success "Server is up at ${HOST}"

# Warmup
print_step "Warming up (${WARMUP}s)"
h2load -n 100 -c 10 -t 2 "$HOST/" > /dev/null 2>&1
print_separator
print_step "TEST 1: Static File Cache Performance"
print_substep "Testing: /index.html (should be cached)"
print_separator
h2load -n 10000 -c 100 -t 4 -D ${DURATION} "$HOST/index.html"
print_separator
print_step "TEST 2: Maximum Throughput (Root Path)"
print_substep "Testing: / (homepage)"
print_separator
h2load -n 50000 -c 100 -t 4 -D ${DURATION} "$HOST/"
print_separator
print_step "TEST 3: High Concurrency (1000 clients)"
print_substep "Testing connection handling"
print_separator
h2load -n 10000 -c 1000 -t 8 -D ${DURATION} "$HOST/index.html"
print_separator
print_step "TEST 4: Small Static Asset (JS/CSS)"
print_substep "Testing: app.js"
print_separator
h2load -n 20000 -c 200 -t 4 -D ${DURATION} "$HOST/app.js"
print_separator
print_step "TEST 5: HTTP/2 Multiplexing (10 streams)"
print_substep "Testing: Multiple concurrent streams per connection"
print_separator
h2load -n 10000 -c 100 -t 4 -m 10 -D ${DURATION} "$HOST/index.html"
print_separator
print_step "TEST 6: Latency Distribution"
print_substep "Testing: p50, p99, p999 latencies"
print_separator
h2load -n 5000 -c 50 -t 2 "$HOST/index.html"

print_separator
print_success "Benchmark complete"
echo ""
print_step "Key metrics to watch"
print_substep "req/s: Requests per second (higher is better)"
print_substep "time for request: Latency (lower is better)"
print_substep "min/mean/max: Response time distribution"
print_substep "p99/p999: Tail latency (should be low if cache works)"
echo ""
print_step "Expected results with v0.0.10"
print_substep "Static files: <1ms p99 (cached)"
print_substep "Throughput: 50,000+ req/s"
print_substep "1000 concurrent: No rejections"
print_substep "Memory: Bounded and stable"
