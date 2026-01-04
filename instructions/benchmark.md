# Benchmark Procedure (AI Agent)

These instructions are for AI agents running benchmarks and producing **analysis-first** reports. Do **not** save raw script output verbatim. Instead, run the commands, **analyze the results**, and write a concise, structured report with your conclusions.

## Output Location & Naming

Write reports to `benchmark/` using these patterns:

| Type | Filename | Header |
|------|----------|--------|
| Full run | `benchmark/full.<TS>.md` | `# full.<TS>` |
| Quick run | `benchmark/quick.<TS>.md` | `# quick.<TS>` |
| Quick asset | `benchmark/quick.app.js.<TS>.md` | `# quick.app.js.<TS>` |

Where `<TS>` = `YYYY-MM-DD-HHMMSS` (UTC)

**Important:** The `# header` must match the filename (without `.md` extension).

## Run All Benchmarks

Execute all three benchmarks in sequence:

```bash
TS=$(date -u +"%Y-%m-%d-%H%M%S")
./scripts/benchmark/benchmark.sh        # Full benchmark
./scripts/benchmark/quick-bench.sh      # Quick benchmark (default)
./scripts/benchmark/quick-bench.sh /app.js  # Quick benchmark (app.js)
```

Then create three report files using the same `$TS` value.

## Full Benchmark Report

**File:** `benchmark/full.${TS}.md`

```markdown
# full.<TS>

Content:
- Test target + host
- Key throughput numbers (req/s)
- Latency summary (p50/p95/p99/p999 if present)
- Error rates or failures
- Notable regressions vs prior runs (if obvious)
- Short conclusion (is this good, bad, or mixed?)
```

## Quick Benchmark Report (default /index.html)

**File:** `benchmark/quick.${TS}.md`

```markdown
# quick.<TS>

Content:
- Target endpoint
- Req/s + latency summary
- Whether results look stable vs recent history
- Short conclusion
```

## Quick Benchmark Report (app.js)

**File:** `benchmark/quick.app.js.${TS}.md`

```markdown
# quick.app.js.<TS>

Content:
- Target endpoint
- Req/s + latency summary
- Any anomalies
- Short conclusion
```

## Update Docs

After creating all reports:

```bash
./scripts/sphinx/dynamics.sh
./scripts/sphinx/build.sh
```
