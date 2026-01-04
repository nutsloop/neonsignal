#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="$ROOT_DIR/build/neonjsx"

mkdir -p "$OUT_DIR"
npx babel "$ROOT_DIR/neonjsx/runtime.ts" --extensions .ts --out-dir "$OUT_DIR" --verbose
