# Plan: Helix as IDE for NeonSignal

## Overview

Configure [Helix](https://helix-editor.com/) as the primary development environment for the NeonSignal project. Helix is a post-modern terminal editor with built-in LSP support, tree-sitter syntax highlighting, and a Kakoune-inspired modal editing model.

## Why Helix

| Feature          | Benefit                                                     |
|------------------|-------------------------------------------------------------|
| Built-in LSP     | No plugin management - language servers work out of the box |
| Tree-sitter      | Fast, accurate syntax highlighting and text objects         |
| Multiple cursors | Native multi-cursor editing (no plugins)                    |
| Modal editing    | Kakoune-style selection-first model (`select → action`)     |
| Terminal-native  | Works over SSH, in tmux, low resource usage                 |
| Rust-based       | Fast startup, no garbage collection pauses                  |

## Project Languages

NeonSignal uses multiple languages that need LSP support:

| Language       | Files                              | LSP Server                   |
|----------------|------------------------------------|------------------------------|
| C++            | `src/**/*.c++`, `include/**/*.h++` | `clangd`                     |
| TypeScript/TSX | `*jsx/**/*.ts`, `*jsx/**/*.tsx`    | `typescript-language-server` |
| Python         | `docs/**/*.py`                     | `pylsp` or `pyright`         |
| Bash           | `scripts/*.sh`                     | `bash-language-server`       |
| Markdown       | `docs/**/*.md`, `plans/*.md`       | `marksman`                   |
| TOML           | `pyproject.toml`, `*.toml`         | `taplo`                      |
| JSON           | `package.json`, `tsconfig.json`    | Built-in                     |
| CSS            | `**/*.css`                         | `vscode-css-language-server` |
| Meson          | `meson.build`                      | (no LSP, tree-sitter only)   |

## Installation

### Helix

```bash
git clone https://github.com/helix-editor/helix && cd helix

# Optimized
cargo install \
   --profile opt \
   --config 'build.rustflags="-C target-cpu=native"' \
   --path helix-term \
   --locked

mkdir -p ~/.config/helix/{runtime, themes}

# Runtime setup (required for Cargo/source builds)
# Copy runtime into ~/.config/helix (recommended)
cp -a ./runtime/. ~/.config/helix/runtime/

# If installed via Cargo, you MUST set up the runtime:
hx --grammar fetch
hx --grammar build

# Verify version
hx --version

```

## Configuration

### Main Config (`~/.config/helix/config.toml`)

```toml
theme = "synthwave"  # Custom theme (see below)

[editor]
line-number = "absolute"
mouse = true
cursorline = true
color-modes = true
true-color = true
bufferline = "multiple"
auto-format = true
auto-save = true
idle-timeout = 400
completion-timeout = 100

[editor.cursor-shape]
insert = "bar"
normal = "block"
select = "underline"

[editor.file-picker]
hidden = false
git-ignore = true
max-depth = 8

[editor.lsp]
display-messages = true
display-inlay-hints = true

[editor.indent-guides]
render = true
character = "│"
skip-levels = 1

[editor.statusline]
left = ["mode", "spinner", "file-name", "file-modification-indicator"]
center = ["diagnostics"]
right = ["selections", "position", "file-encoding", "file-line-ending", "file-type"]
separator = "│"
mode.normal = "NORMAL"
mode.insert = "INSERT"
mode.select = "SELECT"

[editor.whitespace.render]
space = "none"
tab = "all"
newline = "none"

[editor.whitespace.characters]
tab = "→"
tabpad = "·"

[editor.soft-wrap]
enable = true
max-wrap = 25
max-indent-retain = 40

[keys.normal]
# Quick save
"C-s" = ":w"

# Buffer navigation
"C-h" = "jump_view_left"
"C-l" = "jump_view_right"
"C-j" = "jump_view_down"
"C-k" = "jump_view_up"

# Tab/buffer management
"H" = ":buffer-previous"
"L" = ":buffer-next"
"Q" = ":buffer-close"

# LSP shortcuts
"g" = { "d" = "goto_definition", "r" = "goto_reference", "i" = "goto_implementation", "t" = "goto_type_definition" }
"K" = "hover"

# Space leader key (all space-prefixed bindings must be in one table)
[keys.normal.space]
"space" = "file_picker"
"b" = "buffer_picker"
"s" = "symbol_picker"
"S" = "workspace_symbol_picker"
"/" = "global_search"
"r" = "rename_symbol"
"a" = "code_action"
"f" = ":format"
"d" = "diagnostics_picker"
"t" = ":sh bash"

[keys.insert]
"C-s" = ["normal_mode", ":w"]
"C-space" = "completion"
```

### Reload Behavior

Helix does not support an `auto-reload` setting. Use manual reload commands or focus-based prompts when files change on disk.

**Manual reload commands:**

| Command            | Action                |
|--------------------|-----------------------|
| `:reload` or `:r`  | Reload current buffer |
| `:reload-all`      | Reload all buffers    |

**Optional keybindings:**

```toml
[keys.normal]
#"C-r" = ":reload"
"C-r" = ":reload-all"

[keys.normal.space]
"R" = ":reload-all"
```

**Focus-based reload:**

Helix checks for changes when you switch buffers. If an external process edits a file:

1. Switch to another buffer (`H` or `L`)
2. Switch back and Helix will prompt if the file changed

**Why this is manual:**

Helix prompts when a file changes on disk to avoid clobbering unsaved edits.

## Language Servers

```bash
# C++ (clangd from LLVM)
sudo dnf install clang-tools-extra

# TypeScript
npm install -g typescript typescript-language-server

# Python
pip install python-lsp-server

# Bash
npm install -g bash-language-server

# Markdown
wget -O marksman https://github.com/artempyanykh/marksman/releases/download/2025-12-13/marksman-linux-arm64 &&
chmod +x marksman && mv marksman ~/.local/bin

# TOML
cargo install taplo-cli --locked

# CSS/HTML
npm install -g vscode-langservers-extracted
```

### Language Config (`~/.config/helix/languages.toml`)

```toml
# ═══════════════════════════════════════════════════════════════════════════
# C++ Configuration for NeonSignal
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "cpp"
scope = "source.cpp"
file-types = ["cpp", "c++", "cxx", "cc", "h++", "hpp", "hxx", "hh"]
roots = ["meson.build", "compile_commands.json", ".git"]
language-servers = ["clangd"]
auto-format = true
formatter = { command = "clang-format", args = ["--style=file"] }
indent = { tab-width = 2, unit = "  " }
comment-token = "//"

[language-server.clangd]
command = "clangd"
args = [
  "--background-index",
  "--clang-tidy",
  "--completion-style=detailed",
  "--header-insertion=iwyu",
  "--suggest-missing-includes",
  "--cross-file-rename",
  "--pch-storage=memory",
  "-j=4"
]

# ═══════════════════════════════════════════════════════════════════════════
# TypeScript/TSX for NeonJSX
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "typescript"
scope = "source.ts"
file-types = ["ts", "mts", "cts"]
roots = ["package.json", "tsconfig.json"]
language-servers = ["typescript-language-server"]
auto-format = true
indent = { tab-width = 2, unit = "  " }

[[language]]
name = "tsx"
scope = "source.tsx"
file-types = ["tsx"]
roots = ["package.json", "tsconfig.json"]
language-servers = ["typescript-language-server"]
auto-format = true
indent = { tab-width = 2, unit = "  " }

[language-server.typescript-language-server]
command = "typescript-language-server"
args = ["--stdio"]
config = { hostInfo = "helix" }

# ═══════════════════════════════════════════════════════════════════════════
# Python for Sphinx
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "python"
scope = "source.python"
file-types = ["py"]
roots = ["pyproject.toml", "setup.py", "requirements.txt"]
language-servers = ["pylsp"]
auto-format = true
formatter = { command = "black", args = ["-", "--quiet"] }
indent = { tab-width = 4, unit = "    " }

[language-server.pylsp]
command = "pylsp"

# ═══════════════════════════════════════════════════════════════════════════
# Bash for Scripts
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "bash"
scope = "source.bash"
file-types = ["sh", "bash"]
roots = []
language-servers = ["bash-language-server"]
formatter = { command = "shfmt", args = ["-i", "2", "-ci"] }
auto-format = false
indent = { tab-width = 2, unit = "  " }

[language-server.bash-language-server]
command = "bash-language-server"
args = ["start"]

# ═══════════════════════════════════════════════════════════════════════════
# Markdown for Documentation
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "markdown"
scope = "source.md"
file-types = ["md", "markdown"]
roots = [".marksman.toml"]
language-servers = ["marksman"]
soft-wrap.enable = true

[language-server.marksman]
command = "marksman"
args = ["server"]

# ═══════════════════════════════════════════════════════════════════════════
# CSS for Theme Development
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "css"
scope = "source.css"
file-types = ["css"]
language-servers = ["vscode-css-language-server"]
auto-format = true

[language-server.vscode-css-language-server]
command = "vscode-css-language-server"
args = ["--stdio"]

# ═══════════════════════════════════════════════════════════════════════════
# TOML
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "toml"
scope = "source.toml"
file-types = ["toml"]
language-servers = ["taplo"]

[language-server.taplo]
command = "taplo"
args = ["lsp", "stdio"]

# ═══════════════════════════════════════════════════════════════════════════
# JSON
# ═══════════════════════════════════════════════════════════════════════════

[[language]]
name = "json"
scope = "source.json"
file-types = ["json"]
auto-format = true
formatter = { command = "prettier", args = ["--parser", "json"] }
```

```bash

# Verify language servers

hx --health c++
hx --health typescript
hx --health python
hx --health bash
hx --health markdown
```

## Custom Synthwave Theme (`~/.config/helix/themes/synthwave.toml`)

```toml
# ═══════════════════════════════════════════════════════════════════════════
# NeonSignal Synthwave Theme for Helix
# ═══════════════════════════════════════════════════════════════════════════

inherits = "dark_plus"

# Palette
"ui.background" = { bg = "#05010d" }
"ui.background.separator" = { fg = "#2a1f3d" }

"ui.cursor" = { fg = "#05010d", bg = "#00fff9" }
"ui.cursor.match" = { fg = "#05010d", bg = "#ff2a6d" }
"ui.cursor.primary" = { fg = "#05010d", bg = "#00fff9" }
"ui.cursor.select" = { fg = "#05010d", bg = "#9d4edd" }

"ui.cursorline.primary" = { bg = "#120525" }
"ui.cursorline.secondary" = { bg = "#0a0515" }

"ui.linenr" = { fg = "#4a3a6d" }
"ui.linenr.selected" = { fg = "#00fff9", modifiers = ["bold"] }

"ui.statusline" = { fg = "#e0e0e8", bg = "#0a0515" }
"ui.statusline.inactive" = { fg = "#4a3a6d", bg = "#05010d" }
"ui.statusline.normal" = { fg = "#05010d", bg = "#00fff9", modifiers = ["bold"] }
"ui.statusline.insert" = { fg = "#05010d", bg = "#ff2a6d", modifiers = ["bold"] }
"ui.statusline.select" = { fg = "#05010d", bg = "#9d4edd", modifiers = ["bold"] }

"ui.bufferline" = { fg = "#e0e0e8", bg = "#0a0515" }
"ui.bufferline.active" = { fg = "#00fff9", bg = "#120525", modifiers = ["bold"] }

"ui.popup" = { fg = "#e0e0e8", bg = "#0a0515" }
"ui.menu" = { fg = "#e0e0e8", bg = "#0a0515" }
"ui.menu.selected" = { fg = "#00fff9", bg = "#1a0a30" }
"ui.menu.scroll" = { fg = "#9d4edd", bg = "#0a0515" }

"ui.window" = { fg = "#2a1f3d" }
"ui.help" = { fg = "#e0e0e8", bg = "#0a0515" }

"ui.text" = { fg = "#e0e0e8" }
"ui.text.focus" = { fg = "#00fff9" }
"ui.text.inactive" = { fg = "#4a3a6d" }

"ui.virtual.whitespace" = { fg = "#2a1f3d" }
"ui.virtual.indent-guide" = { fg = "#2a1f3d" }
"ui.virtual.ruler" = { bg = "#120525" }
"ui.virtual.inlay-hint" = { fg = "#4a3a6d", modifiers = ["italic"] }

"ui.selection" = { bg = "#2a1f4d" }
"ui.selection.primary" = { bg = "#3a2f6d" }

# Syntax highlighting
"attribute" = { fg = "#9d4edd" }
"comment" = { fg = "#6a5a8d", modifiers = ["italic"] }
"constant" = { fg = "#9d4edd" }
"constant.numeric" = { fg = "#9d4edd" }
"constant.character.escape" = { fg = "#ff2a6d" }

"constructor" = { fg = "#00fff9" }

"function" = { fg = "#00ff88" }
"function.builtin" = { fg = "#00ff88", modifiers = ["italic"] }
"function.macro" = { fg = "#ff2a6d" }
"function.method" = { fg = "#00ff88" }

"keyword" = { fg = "#ff2a6d" }
"keyword.control" = { fg = "#ff2a6d" }
"keyword.control.return" = { fg = "#ff2a6d", modifiers = ["bold"] }
"keyword.control.import" = { fg = "#9d4edd" }
"keyword.function" = { fg = "#ff2a6d" }
"keyword.operator" = { fg = "#ff2a6d" }
"keyword.storage" = { fg = "#ff2a6d" }
"keyword.storage.type" = { fg = "#00fff9" }
"keyword.storage.modifier" = { fg = "#ff2a6d" }

"label" = { fg = "#f9f002" }

"namespace" = { fg = "#e0e0e8" }

"operator" = { fg = "#ff2a6d" }

"punctuation" = { fg = "#e0e0e8" }
"punctuation.bracket" = { fg = "#e0e0e8" }
"punctuation.delimiter" = { fg = "#e0e0e8" }
"punctuation.special" = { fg = "#ff2a6d" }

"special" = { fg = "#ff2a6d" }

"string" = { fg = "#f9f002" }
"string.regexp" = { fg = "#ff2a6d" }
"string.special" = { fg = "#f9f002", modifiers = ["bold"] }

"tag" = { fg = "#ff2a6d" }
"tag.attribute" = { fg = "#00fff9" }

"type" = { fg = "#00fff9" }
"type.builtin" = { fg = "#00fff9", modifiers = ["italic"] }
"type.enum" = { fg = "#00fff9" }
"type.enum.variant" = { fg = "#9d4edd" }

"variable" = { fg = "#e0e0e8" }
"variable.builtin" = { fg = "#ff2a6d" }
"variable.parameter" = { fg = "#ffaa00" }
"variable.other.member" = { fg = "#00fff9" }

# Markup (Markdown)
"markup.heading" = { fg = "#00fff9", modifiers = ["bold"] }
"markup.heading.1" = { fg = "#ff2a6d", modifiers = ["bold"] }
"markup.heading.2" = { fg = "#00fff9", modifiers = ["bold"] }
"markup.heading.3" = { fg = "#9d4edd", modifiers = ["bold"] }
"markup.bold" = { modifiers = ["bold"] }
"markup.italic" = { modifiers = ["italic"] }
"markup.link.url" = { fg = "#00fff9", modifiers = ["underlined"] }
"markup.link.text" = { fg = "#ff2a6d" }
"markup.raw" = { fg = "#f9f002" }
"markup.raw.inline" = { fg = "#f9f002" }
"markup.raw.block" = { fg = "#f9f002" }
"markup.list" = { fg = "#ff2a6d" }
"markup.quote" = { fg = "#9d4edd", modifiers = ["italic"] }

# Diff
"diff.plus" = { fg = "#00ff88" }
"diff.minus" = { fg = "#ff4466" }
"diff.delta" = { fg = "#f9f002" }

# Diagnostics
"diagnostic.error" = { underline = { color = "#ff4466", style = "curl" } }
"diagnostic.warning" = { underline = { color = "#ffaa00", style = "curl" } }
"diagnostic.info" = { underline = { color = "#00fff9", style = "curl" } }
"diagnostic.hint" = { underline = { color = "#9d4edd", style = "curl" } }

"error" = { fg = "#ff4466" }
"warning" = { fg = "#ffaa00" }
"info" = { fg = "#00fff9" }
"hint" = { fg = "#9d4edd" }

# Git gutter
"diff.plus.gutter" = { fg = "#00ff88" }
"diff.minus.gutter" = { fg = "#ff4466" }
"diff.delta.gutter" = { fg = "#f9f002" }
```

## Essential Keybindings Reference

### Navigation

| Key           | Action                       |
|---------------|------------------------------|
| `Space Space` | File picker                  |
| `Space b`     | Buffer picker                |
| `Space s`     | Symbol picker (current file) |
| `Space S`     | Workspace symbol picker      |
| `Space /`     | Global search (ripgrep)      |
| `gd`          | Go to definition             |
| `gr`          | Go to references             |
| `gi`          | Go to implementation         |
| `gt`          | Go to type definition        |
| `Ctrl-o`      | Jump back                    |
| `Ctrl-i`      | Jump forward                 |

### Editing

| Key       | Action                             |
|-----------|------------------------------------|
| `K`       | Hover documentation                |
| `Space r` | Rename symbol                      |
| `Space a` | Code actions                       |
| `Space f` | Format document                    |
| `Space d` | Diagnostics picker                 |
| `x`       | Select line                        |
| `s`       | Select inside text object          |
| `mm`      | Match brackets                     |
| `C`       | Copy selection down (multi-cursor) |
| `Alt-C`   | Copy selection up (multi-cursor)   |
| `,`       | Remove other cursors               |

### LSP

| Key          | Action              |
|--------------|---------------------|
| `Ctrl-Space` | Trigger completion  |
| `Tab`        | Next completion     |
| `Shift-Tab`  | Previous completion |
| `Enter`      | Accept completion   |

### Buffers/Windows

| Key            | Action           |
|----------------|------------------|
| `H`            | Previous buffer  |
| `L`            | Next buffer      |
| `Q`            | Close buffer     |
| `Ctrl-w v`     | Vertical split   |
| `Ctrl-w s`     | Horizontal split |
| `Ctrl-h/j/k/l` | Navigate splits  |

## Workflow Tips

### 1. Multi-Cursor Editing

```
# Select word, then add more cursors
1. `w` to select word
2. `*` to select all occurrences
3. `c` to change all at once
```

### 2. Tree-sitter Text Objects

```
# Select inside function
1. Position cursor inside function
2. `mif` (match inside function)

# Select around class
1. `mac` (match around class)
```

### 3. LSP Diagnostics Workflow

```
# Jump through errors
1. `]d` next diagnostic
2. `[d` previous diagnostic
3. `Space d` open diagnostics picker
4. `Space a` apply code action fix
```

### 4. Git Integration

```
# View diff hunks
1. `]g` next git hunk
2. `[g` previous git hunk
3. `Space g` (if configured) git commands
```

## Troubleshooting

### clangd Not Finding Headers

```bash
# Ensure compile_commands.json exists
ls -la /home/core/code/neonsignal/compile_commands.json

# If missing, regenerate
cd /home/core/code/neonsignal
meson setup build --wipe
ln -sf build/compile_commands.json .
```

### TypeScript LSP Slow

```bash
# Check for large node_modules
# Add to .helix/languages.toml:
[language-server.typescript-language-server]
config.preferences.includePackageJsonAutoImports = "off"
```

### Theme Not Loading

```bash
# Check theme location
ls ~/.config/helix/themes/synthwave.toml

# Reload with
:theme synthwave
```

## Resources

- [Helix Documentation](https://docs.helix-editor.com/)
- [Helix Wiki](https://github.com/helix-editor/helix/wiki)
- [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)
- [Tree-sitter](https://tree-sitter.github.io/)
