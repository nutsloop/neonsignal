# full.2026-01-06-085253

Content:
- Test target + host: https://10.0.0.10:9443 (HTTP/2, TLSv1.3)
- Key throughput numbers (req/s):
  - /index.html (cached): ~8,886 req/s
  - / (root): ~9,778 req/s
  - High concurrency (1000 clients): ~7,201 req/s
  - app.js (small asset): ~9,716 req/s
  - HTTP/2 multiplexing (10 streams): ~9,749 req/s
  - Latency distribution run (5k req): ~8,191 req/s
- Latency summary (mean/min/max):
  - /index.html: mean 11.16ms (min 1.16ms, max 98.85ms)
  - /: mean 10.14ms (min 2.44ms, max 99.36ms)
  - 1000 clients: mean 131.43ms (min 101.20ms, max 954.17ms)
  - app.js: mean 20.34ms (min 0.455ms, max 188.39ms)
  - multiplexing: mean 101.42ms (min 79.44ms, max 187.73ms)
  - latency run: mean 5.41ms (min 0.182ms, max 70.09ms)
- Error rates or failures: 0 failed / 0 errored / 0 timeout across all tests
- Notable regressions vs prior runs (if obvious): no prior baseline recorded in this run
- Short conclusion: Strong throughput for cached/root/static assets with clean error rates. High-concurrency and multiplexing tests show expected latency inflation but remain stable without failures.
