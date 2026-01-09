#!/usr/bin/env bash

# Centralized repository paths (resolved relative to this script).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NEONSIGNAL_ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
NEONSIGNAL_SCRIPTS_DIR="$NEONSIGNAL_ROOT_DIR/scripts"
NEONSIGNAL_LOGGING_LIB_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/lib/logging.sh"

# Default vhost pipeline (static site)
NEONSIGNAL_DEFAULT_SOURCE_DIR="$NEONSIGNAL_ROOT_DIR/www/_default" # source directory
NEONSIGNAL_DEFAULT_PUBLIC_DIR="$NEONSIGNAL_ROOT_DIR/public/_default" # public output
# Default vhost scripts
NEONSIGNAL_DEFAULT_ALL_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/_default/all.sh" # pipeline orchestrator
NEONSIGNAL_DEFAULT_BUILD_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/_default/build.sh"
NEONSIGNAL_DEFAULT_CLEAN_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/_default/clean.sh"

# Shared NeonJSX runtime
NEONSIGNAL_NEONJSX_SOURCE_DIR="$NEONSIGNAL_ROOT_DIR/neonjsx" # runtime source
NEONSIGNAL_NEONJSX_BUILD_DIR="$NEONSIGNAL_ROOT_DIR/build/neonjsx" # transpiled runtime
# NeonJSX scripts
NEONSIGNAL_NEONJSX_ALL_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/neonjsx/all.sh" # pipeline orchestrator
NEONSIGNAL_NEONJSX_BUILD_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/neonjsx/build.sh"
NEONSIGNAL_NEONJSX_CLEAN_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/neonjsx/clean.sh"

# ESLint scripts
NEONSIGNAL_ESLINT_LINT_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/eslint/lint.sh"
NEONSIGNAL_ESLINT_LINT_FIX_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/eslint/lint-fix.sh"

# Certificate management
NEONSIGNAL_CERTS_DIR="$NEONSIGNAL_ROOT_DIR/certs"
NEONSIGNAL_CERTS_CA_DIR="$NEONSIGNAL_CERTS_DIR/ca"
# Certificate scripts
NEONSIGNAL_CERT_ISSUER_SCRIPT="$NEONSIGNAL_SCRIPTS_DIR/certificates/issuer.sh"
