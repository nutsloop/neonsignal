#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

have_python_packages() {
  python - "$@" <<'PY'
import importlib.util
import sys

missing = [name for name in sys.argv[1:] if importlib.util.find_spec(name) is None]
if missing:
    sys.exit(1)
PY
}

PIP_FLAGS=(--disable-pip-version-check -qq)

print_header "NeonSignal Sphinx Setup"
print_step "Setting up Sphinx environment"

mkdir -p "$NEONSIGNAL_SPHINX_BOOK_DIR/source"

if [[ -d "$NEONSIGNAL_SPHINX_VENV_DIR" ]]; then
  print_substep "Using existing venv at ${NEONSIGNAL_SPHINX_VENV_DIR}"
else
  python3 -m venv "$NEONSIGNAL_SPHINX_VENV_DIR"
fi

if [[ "${VIRTUAL_ENV:-}" == "$NEONSIGNAL_SPHINX_VENV_DIR" ]]; then
  print_substep "Venv already activated"
else
  source "$NEONSIGNAL_SPHINX_VENV_DIR/bin/activate"
fi

print_substep "Ensuring core Python tooling"
if have_python_packages pip setuptools wheel; then
  print_substep "Core tooling already present"
else
  python -m pip install "${PIP_FLAGS[@]}" -U pip setuptools wheel
fi

print_substep "Ensuring Sphinx and extensions"
if have_python_packages sphinx myst_parser sphinx_copybutton sphinx_design; then
  print_substep "Sphinx and extensions already present"
else
  python -m pip install "${PIP_FLAGS[@]}" sphinx myst-parser sphinx-copybutton sphinx-design
fi

if [[ -d "$NEONSIGNAL_SPHINX_THEME_DIR" ]]; then
  if have_python_packages sphinx_synthwave_theme; then
    print_substep "Synthwave theme already present"
  else
    print_substep "Installing synthwave theme from ${NEONSIGNAL_SPHINX_THEME_DIR}"
    python -m pip install "${PIP_FLAGS[@]}" -e "$NEONSIGNAL_SPHINX_THEME_DIR"
    print_success "Theme installed"
  fi
else
  print_warning "Synthwave theme not found at ${NEONSIGNAL_SPHINX_THEME_DIR}"
fi
python -m pip freeze > "$NEONSIGNAL_SPHINX_BOOK_DIR/requirements.txt"
