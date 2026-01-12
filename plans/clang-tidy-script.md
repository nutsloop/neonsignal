# Plan: clang-tidy Script for NeonSignal

## Request

Create a build script that runs clang-tidy over the `src/` and `include/` trees.

Flags:
- `--check` → check linting only
- `--fix` → check and then apply fixes

Requirements:
- Save script as `scripts/build/clang-tidy.sh`
- Make it executable
- Source `scripts/global_variables.sh`
- Use helpers in `scripts/lib/logging.sh`

## Implementation Notes

- Use `build/compile_commands.json` as the clang-tidy compilation database.
- Discover `*.c++` and `*.h++` files in `src/` and `include/`.
- Fail fast if `clang-tidy` is missing or if the compilation database is absent.
- For `--check`, run with `-warnings-as-errors='*'` and exit non-zero on findings.
- For `--fix`, run a check first, then apply `clang-tidy -fix -format-style=file`.
