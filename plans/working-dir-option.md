# Plan: Add `--working-dir=<path>` CLI Option

## Overview
Add a `--working-dir` option that changes the process working directory before other paths are resolved. Uses `std::filesystem::current_path(path)` which is portable across Linux and macOS (C++17).

## Critical Timing
The working directory must be changed **AFTER** help/version checks but **BEFORE** ServerConfig is populated, so relative paths like `./public`, `./certs`, `./data` resolve correctly.

## Additional Requirements
- Add `working_dir` to `ServerConfig` struct
- Log the actual working directory via `std::cerr` at server boot

## Files Created

### 1. `src/neonsignal/voltage_argv/check/working_dir.c++`
- Validates working directory exists, is a directory, and is writable
- Uses `access(path, W_OK)` to verify write permissions
- Supports `NEONSIGNAL_WORKING_DIR` environment variable
- No default (current directory used if not specified)

### 2. `src/neonsignal/voltage_argv/help/working_dir_.c++`
- Standard help format with NAME, DESCRIPTION, ENVIRONMENT, EXAMPLES
- Environment variable: `NEONSIGNAL_WORKING_DIR`

### 3. `src/neonsignal/server_voltage/working_dir.c++`
- Simple accessor returning `working_dir_` member

## Files Modified

### 1. `include/neonsignal/voltage_argv.h++`
- Added `[[nodiscard]] const std::optional<std::string> &working_dir() const;` accessor
- Added `std::optional<std::string> working_dir_;` member

### 2. `include/neonsignal/voltage_argv/check.h++`
- Added `[[nodiscard]] std::string working_dir() const;` declaration

### 3. `include/neonsignal/voltage_argv/help.h++`
- Added `working_dir` to `Topic_` enum (server options section)
- Added `[[nodiscard]] std::string working_dir_() const;` declaration

### 4. `src/neonsignal/voltage_argv.c++`
- Added `"working-dir"` to `server_args_list`
- Added `"working-dir"` to `server_skip_digits()`
- Added parsing logic for working-dir option

### 5. `src/neonsignal/voltage_argv/help/get_help_topic_.c++`
- Added `case Topic_::working_dir: return working_dir_();`

### 6. `src/neonsignal/voltage_argv/help/help_.c++`
- Added option line for `--working-dir=<path>`
- Added `NEONSIGNAL_WORKING_DIR` to environment variables list

### 7. `src/neonsignal/voltage_argv/help/set_option_list_.c++`
- Added `{"working-dir", Topic_::working_dir}` to server options

### 8. `src/main.c++`
- Added `#include <filesystem>` and `#include <format>`
- Added working directory change logic after help/version checks, before ServerConfig
- Stores resolved path in `config.working_dir`

### 9. `include/neonsignal/neonsignal.h++`
- Added `std::string working_dir;` to `ServerConfig` struct

### 10. `src/neonsignal/http2_listener/start.c++`
- Added `#include <filesystem>`
- Added working directory log before "Preloading" message

### 11. `src/meson.build`
- Added `check/working_dir.c++` (both targets)
- Added `help/working_dir_.c++` (both targets)
- Added `server_voltage/working_dir.c++` (main target only)

## Verification

1. Build: `meson compile -C build`
2. Test help: `./build/src/neonsignal --help=working-dir`
3. Test functionality:
   ```bash
   mkdir -p /tmp/test-workdir/public /tmp/test-workdir/certs /tmp/test-workdir/data
   ./build/src/neonsignal --working-dir=/tmp/test-workdir spin
   ```
4. Test error handling: `./build/src/neonsignal --working-dir=/nonexistent spin`
