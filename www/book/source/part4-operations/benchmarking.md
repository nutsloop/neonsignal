# Benchmarking

The Neonsignal repository includes scripts to help you benchmark the server's performance.

## Quick Benchmark

For rapid, ad-hoc testing, you can use the `scripts/quick-bench.sh` script. It runs a short, 5-second `h2load` test against a specified path.

**Usage:**

```bash
# Test the cached homepage
./scripts/quick-bench.sh /index.html

# Test an API endpoint
./scripts/quick-bench.sh /api/stats
```

## Comprehensive Benchmark Suite

For a more thorough performance analysis, the `scripts/benchmark.sh` script runs a series of `h2load` tests designed to measure different aspects of the server's performance.

The suite includes the following tests:

1.  **Static File Cache:** Measures performance for a file that should be in the in-memory cache.
2.  **Maximum Throughput:** Tests the server's request-handling capacity on the root path.
3.  **High Concurrency:** Simulates 1,000 concurrent clients to test connection management.
4.  **Small Static Asset:** Benchmarks the delivery of typical web assets like JavaScript or CSS files.
5.  **HTTP/2 Multiplexing:** Tests the server's ability to handle multiple concurrent streams on each connection.
6.  **Latency Distribution:** Provides detailed latency statistics (p50, p99, p999).

### Usage

Before running the benchmark, ensure the server is running and accessible at the `HOST` address defined in the script.

```bash
./scripts/benchmark.sh
```

### Expected Results

With the performance improvements introduced in version 0.0.10, you should expect to see results similar to the following:

-   **Static files:** Sub-millisecond p99 latency, indicating that the content is being served from the in-memory cache.
-   **Throughput:** 50,000+ requests per second.
-   **High Concurrency:** No rejected connections, demonstrating the effectiveness of the `ConnectionManager`.
-   **Memory Usage:** The server's memory usage should remain bounded and stable throughout the test.
