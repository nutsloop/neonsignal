#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../global_variables.sh"
source "$NEONSIGNAL_LOGGING_LIB_SCRIPT"

print_header "NeonSignal Sphinx Build"

if [[ ! -d "$NEONSIGNAL_SPHINX_VENV_DIR" ]]; then
  print_error "Missing venv. Run scripts/sphinx/setup.sh first."
  exit 1
fi

print_step "Activating sphinx environment"
source "$NEONSIGNAL_SPHINX_VENV_DIR/bin/activate"

print_step "Building book site"
print_substep "Build output: ${NEONSIGNAL_SPHINX_BUILD_DIR}"
mkdir -p "$NEONSIGNAL_SPHINX_BUILD_DIR"
python -m sphinx -q -b html "$NEONSIGNAL_SPHINX_BOOK_DIR/source" "$NEONSIGNAL_SPHINX_BUILD_DIR"
print_success "Sphinx book build complete"

print_step "Deploying book to public"
print_substep "Destination: ${NEONSIGNAL_SPHINX_PUBLIC_DIR}"
mkdir -p "$NEONSIGNAL_SPHINX_PUBLIC_DIR"
rsync --archive --delete "${NEONSIGNAL_SPHINX_BUILD_DIR}/" "${NEONSIGNAL_SPHINX_PUBLIC_DIR}/"
print_success "book deployed to ${NEONSIGNAL_SPHINX_PUBLIC_DIR}"

cat > "$NEONSIGNAL_SPHINX_PUBLIC_ENTRY" <<'EOF'
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta http-equiv="refresh" content="0; url=/book/index.html" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Neonsignal Book</title>
    <style>
      body {
        margin: 0;
        min-height: 100vh;
        display: grid;
        place-items: center;
        background: #07070b;
        color: #d9e6ff;
        font-family: "Fira Code", "JetBrains Mono", "SF Mono", monospace;
      }
      a {
        color: #6fffe9;
        text-decoration: none;
      }
    </style>
  </head>
  <body>
    <p>Loading documentation… <a href="/book/index.html">Open docs</a></p>
  </body>
</html>
EOF
print_success "Wrote book entry page @ -> ${NEONSIGNAL_SPHINX_PUBLIC_ENTRY}"
