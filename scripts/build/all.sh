#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Build Script (macOS, Debug)
# Builds backend with debug info and the _default frontend only.
#
# Usage:
#   ./scripts/build/all.sh
#   ./scripts/build/all.sh --delete-build
#   ./scripts/build/all.sh --reset-db
#   ./scripts/build/all.sh --delete-build --reset-db
# ═══════════════════════════════════════════════════════════════════════════

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

DELETE_BUILD=false
RESET_DB=false

OS_NAME="$(uname -s)"
ARCH_NAME="$(uname -m)"
PKG_CONFIG_CMD=""
PKG_CONFIG_PATH_SET=false

command_exists() {
  command -v "$1" >/dev/null 2>&1
}

brew_has_formula() {
  command_exists brew && brew list --versions "$1" >/dev/null 2>&1
}

detect_brew_prefix() {
  if [[ -n "${NEONSIGNAL_BREW_PREFIX:-}" ]]; then
    echo "${NEONSIGNAL_BREW_PREFIX}"
    return
  fi
  if command_exists brew; then
    brew --prefix
    return
  fi
  case "${ARCH_NAME}" in
    arm64)
      echo "/opt/homebrew"
      ;;
    x86_64)
      echo "/usr/local"
      ;;
    *)
      echo "/usr/local"
      ;;
  esac
}

ensure_command_or_brew() {
  local cmd="$1"
  local formula="$2"

  if command_exists "$cmd"; then
    return 0
  fi

  if command_exists brew; then
    if brew_has_formula "$formula"; then
      print_error "Command '${cmd}' not found in PATH but brew formula '${formula}' is installed."
      print_substep "Check your PATH or run: eval \"\\$(brew shellenv)\""
    else
      print_error "Missing command '${cmd}'."
      print_substep "Install with: brew install ${formula}"
    fi
    return 1
  fi

  print_error "Missing command '${cmd}'."
  print_substep "Install it or add it to PATH."
  return 1
}

ensure_pkg_config_cmd() {
  if command_exists pkg-config; then
    PKG_CONFIG_CMD="pkg-config"
    export PKG_CONFIG="${PKG_CONFIG_CMD}"
    return 0
  fi
  if command_exists pkgconf; then
    PKG_CONFIG_CMD="pkgconf"
    export PKG_CONFIG="${PKG_CONFIG_CMD}"
    return 0
  fi
  PKG_CONFIG_CMD="pkg-config"
  ensure_command_or_brew pkg-config pkg-config
}

setup_pkg_config_path() {
  if [[ "${PKG_CONFIG_PATH_SET}" == "true" ]]; then
    return
  fi
  local brew_prefix
  brew_prefix="$(detect_brew_prefix)"
  local paths=()
  local opt_dir
  opt_dir="${brew_prefix}/opt/openssl@3/lib/pkgconfig"
  if [[ -d "${opt_dir}" ]]; then
    paths+=("${opt_dir}")
  fi
  opt_dir="${brew_prefix}/opt/nghttp2/lib/pkgconfig"
  if [[ -d "${opt_dir}" ]]; then
    paths+=("${opt_dir}")
  fi
  opt_dir="${brew_prefix}/opt/libmdbx/lib/pkgconfig"
  if [[ -d "${opt_dir}" ]]; then
    paths+=("${opt_dir}")
  fi
  if [[ -d "${brew_prefix}/lib/pkgconfig" ]]; then
    paths+=("${brew_prefix}/lib/pkgconfig")
  fi
  if [[ "${brew_prefix}" != "/usr/local" && -d "/usr/local/lib/pkgconfig" ]]; then
    paths+=("/usr/local/lib/pkgconfig")
  fi
  if [[ "${brew_prefix}" != "/opt/homebrew" && -d "/opt/homebrew/lib/pkgconfig" ]]; then
    paths+=("/opt/homebrew/lib/pkgconfig")
  fi
  if [[ ${#paths[@]} -gt 0 ]]; then
    local joined
    joined="$(IFS=:; echo "${paths[*]}")"
    export PKG_CONFIG_PATH="${joined}${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}"
  fi
  PKG_CONFIG_PATH_SET=true
}

check_pkg_config_lib() {
  local lib="$1"
  if "$PKG_CONFIG_CMD" --exists "$lib"; then
    return 0
  fi
  setup_pkg_config_path
  if "$PKG_CONFIG_CMD" --exists "$lib"; then
    return 0
  fi
  return 1
}

preflight_checks() {
  if [[ "${OS_NAME}" != "Darwin" ]]; then
    print_error "This build script is macOS-only. Detected: ${OS_NAME}"
    exit 1
  fi

  local missing=false

  ensure_pkg_config_cmd || missing=true
  setup_pkg_config_path
  ensure_command_or_brew meson meson || missing=true
  ensure_command_or_brew ninja ninja || missing=true
  ensure_command_or_brew cmake cmake || missing=true
  ensure_command_or_brew npm node || missing=true

  if [[ "${missing}" == "true" ]]; then
    print_error "Missing required build tools. Aborting."
    exit 1
  fi

  if ! check_pkg_config_lib openssl; then
    print_error "pkg-config could not find openssl."
    print_substep "Install openssl@3 or set PKG_CONFIG_PATH."
    missing=true
  fi
  if ! check_pkg_config_lib libnghttp2; then
    print_error "pkg-config could not find libnghttp2."
    print_substep "Install nghttp2 or set PKG_CONFIG_PATH."
    missing=true
  fi
  if ! check_pkg_config_lib libmdbx; then
    print_warning "libmdbx not found via pkg-config; Meson will fetch the subproject."
  fi

  if [[ "${missing}" == "true" ]]; then
    exit 1
  fi
}

patch_libmdbx_subproject() {
  local subproject_dir="${NEONSIGNAL_ROOT_DIR}/subprojects/libmdbx"
  local cmake_file="${subproject_dir}/CMakeLists.txt"

  if [[ ! -d "${subproject_dir}" ]]; then
    print_step "Fetching libmdbx subproject"
    if ! (cd "${NEONSIGNAL_ROOT_DIR}" && meson subprojects download libmdbx); then
      if [[ -f "${cmake_file}" ]]; then
        print_warning "Meson reported an error fetching libmdbx, but CMakeLists.txt is present."
      else
        print_error "Failed to fetch libmdbx subproject."
        exit 1
      fi
    fi
  fi

  if [[ ! -f "${cmake_file}" ]]; then
    print_warning "libmdbx CMakeLists.txt not found; skipping patch"
    return
  fi

  if grep -q "HEADER_FILE_ONLY ON" "${cmake_file}"; then
    return
  fi

  print_step "Patching libmdbx CMakeLists.txt for macOS"
  NEONSIGNAL_ROOT_DIR="${NEONSIGNAL_ROOT_DIR}" python3 - <<'PY'
import os
from pathlib import Path

root = os.environ.get("NEONSIGNAL_ROOT_DIR")
if not root:
    raise SystemExit("libmdbx patch failed: NEONSIGNAL_ROOT_DIR not set")
cmake_file = Path(root) / "subprojects" / "libmdbx" / "CMakeLists.txt"
text = cmake_file.read_text()
needle = 'list(APPEND LIBMDBX_SOURCES "${MDBX_SOURCE_DIR}/mdbx.c++" mdbx.h++)'
if needle not in text:
    raise SystemExit("libmdbx patch failed: expected CMakeLists.txt pattern not found")
replacement = '\n'.join([
    'list(APPEND LIBMDBX_SOURCES "${MDBX_SOURCE_DIR}/mdbx.c++")',
    '  set_source_files_properties(mdbx.h++ PROPERTIES HEADER_FILE_ONLY ON)',
])
cmake_file.write_text(text.replace(needle, replacement))
PY
  print_success "libmdbx patch applied"
}

show_help() {
  print_header "NEONSIGNAL BUILD (macOS, DEBUG)"
  echo -e "${BOLD}Usage:${RESET}"
  echo -e "  ${CYAN}./scripts/build/all.sh${RESET} [--delete-build] [--reset-db]"
  echo ""
  echo -e "${BOLD}Options:${RESET}"
  echo -e "  ${CYAN}--delete-build${RESET}   Remove build/ before configuring"
  echo -e "  ${CYAN}--reset-db${RESET}       Remove data/ before building"
  echo -e "  ${CYAN}--preflight${RESET}      Run dependency checks only"
  echo ""
}

# ─────────────────────────────────────────────────────────────────────────────
# Parse Arguments
# ─────────────────────────────────────────────────────────────────────────────
for arg in "$@"; do
  case "$arg" in
    --delete-build)
      DELETE_BUILD=true
      ;;
    --reset-db)
      RESET_DB=true
      ;;
    --preflight)
      preflight_checks
      print_success "Preflight checks passed"
      exit 0
      ;;
    --help|-h)
      show_help
      exit 0
      ;;
    *)
      print_error "Unknown option: $arg"
      show_help
      exit 1
      ;;
  esac
done

# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────
print_header "NEONSIGNAL BUILD (macOS, DEBUG)"
preflight_checks
patch_libmdbx_subproject

cd "$NEONSIGNAL_ROOT_DIR"

# Optional DB reset
if [[ "$RESET_DB" == "true" ]]; then
  print_separator
  print_warning "Resetting database directory"
  rm -rf data
  mkdir -p data
  print_success "Database directory reset"
fi

# Configure Meson (debug only)
print_separator
print_step "Configuring Meson for debug build"
if [[ "$DELETE_BUILD" == "true" ]]; then
  print_warning "Removing build directory"
  rm -rf build
fi
if [ -d "build" ]; then
  print_substep "Reconfiguring existing build"
  meson configure build -Dbuildtype=debug -Ddebug=true -Doptimization=0
else
  print_substep "Setting up new build"
  meson setup build --buildtype=debug -Ddebug=true -Doptimization=0
fi
print_success "Meson configured"

# Clean rebuild
print_separator
print_step "Clean rebuild"
print_substep "Ensures all targets use debug settings"
meson compile -C build --clean
print_success "Build cleaned"

# Compile C++
print_separator
print_step "Compiling C++ ${DIM}(debug)${RESET}"
meson compile -C build
print_success "Compilation complete"

echo ""
print_substep "Binaries:"
ls -lh build/src/neonsignal build/src/neonsignal_redirect | while read -r line; do
  echo -e "    ${DIM}│${RESET} ${CYAN}${line}${RESET}"
done

# Build frontend (_default only)

print_separator
print_step "Running npm install"
if ! npm install; then
  print_error "npm install failed"
  print_substep "Check npm output above and ensure Node.js is installed."
  exit 1
fi
print_success "npm install complete"



print_separator
print_step "Building frontend ${DIM}(_default only)${RESET}"
"$NEONSIGNAL_DEFAULT_CLEAN_SCRIPT" || true
"$NEONSIGNAL_DEFAULT_BUILD_SCRIPT"
print_success "Frontend built"

# Summary
print_separator
echo ""
echo -e "${GREEN}${BOLD}◆ Build complete (macOS debug)${RESET}"
echo ""
print_substep "Artifacts:"
echo -e "    ${DIM}│${RESET} ${CYAN}build/src/neonsignal${RESET}"
echo -e "    ${DIM}│${RESET} ${CYAN}build/src/neonsignal_redirect${RESET}"
echo -e "    ${DIM}│${RESET} ${CYAN}${NEONSIGNAL_DEFAULT_PUBLIC_DIR}${RESET}"
echo ""
