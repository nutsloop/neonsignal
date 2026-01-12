# Plan: clang-format Script for NeonSignal

## Request

Create a build script that runs clang-format over the `src/` and `include/` trees.

Flags:
- `--check` → check formatting only
- `--fix` → check and then apply fixes

Requirements:
- Save script as `scripts/build/clang-format.sh`
- Make it executable
- Source `scripts/global_variables.sh`
- Use helpers in `scripts/lib/logging.sh`

## Implementation Notes

- Discover `*.c++` and `*.h++` files in `src/` and `include/`.
- Fail if `clang-format` is missing.
- For `--check`, exit non-zero on formatting issues.
- For `--fix`, run a check and then apply `clang-format -i`.
