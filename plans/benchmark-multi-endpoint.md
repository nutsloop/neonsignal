# Benchmark Multi-Endpoint Plan

Goal: Run max-concurrency h2load benchmarks across HTTPS vhosts, HTTP redirect,
static assets (CSS/JS), a 404 path, and auth-required APIs while tailing
service logs.

1) Inventory assets and endpoints
   - Scan public/ for CSS/JS files per vhost.
   - Confirm auth-required API paths from include/neonsignal/routes.h++.

2) Prepare automation
   - Add a multi-endpoint benchmark script under scripts/benchmark/.
   - Use max clients, max streams, and max threads for all runs.
   - Use h2load --connect-to + --sni to bind domains to the server IP.
   - Include HTTP redirect tests on port 9090 with --h1.

3) Log capture
   - Run sudo journalctl -f for neonsignal and neonsignal-redirect in a
     separate background process while benchmarks run.

4) Execute benchmark runs
   - Run the multi-endpoint script for:
     - simonedelpopolo.host (CSS/JS/404)
     - nutsloop.host (CSS/JS/404)
     - neonsignal.nutsloop.host (CSS/JS/404 + auth-required APIs)
     - http redirect on port 9090

5) Reporting
   - Summarize results (req/s, latency, errors) in benchmark/*.md
     using the timestamped format from instructions/benchmark.md.
   - Ask before running Sphinx dynamics/build per repo policy.
