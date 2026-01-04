# Plan: Synthwave Sphinx Theme

## Overview

Create a custom Sphinx theme with synthwave/wasteland aesthetic that matches the NeonSignal project identity.

## Requirements

- **Responsive design** - Mobile-first, works on all screen sizes
- **Theme modes** - Dark / Light / System preference detection
- **Pip installable** - Standalone package in `tools/sphinx-synthwave-theme/`
- **Sphinx 8.x compatible** - Works with current Sphinx version
- **Preserve functionality** - Keep myst_parser, sphinx_copybutton, sphinx_design support

## Current Setup

| Item | Value |
|------|-------|
| Current theme | `furo` (v2025.12.19) |
| Sphinx version | 8.2.3 |
| Extensions | myst_parser, sphinx_copybutton, sphinx_design |
| Config location | `docs/source/conf.py` |

## Color Palette

### Dark Mode (Primary)

```css
:root[data-theme="dark"] {
  /* Background layers */
  --bg-deep: #0a0a0f;           /* Deepest background */
  --bg-surface: #12121a;        /* Card/surface background */
  --bg-elevated: #1a1a25;       /* Elevated elements */

  /* Synthwave accent colors */
  --neon-cyan: #00ffff;         /* Primary accent */
  --neon-magenta: #ff00ff;      /* Secondary accent */
  --neon-pink: #ff1493;         /* Hot pink highlights */
  --neon-purple: #9d4edd;       /* Purple accents */
  --neon-yellow: #ffff00;       /* Warning/attention */

  /* Text colors */
  --text-primary: #e0e0e0;      /* Main text */
  --text-secondary: #a0a0a0;    /* Muted text */
  --text-dim: #606070;          /* Very muted */

  /* Functional colors */
  --success: #00ff88;           /* Green glow */
  --warning: #ffaa00;           /* Amber */
  --error: #ff4466;             /* Red alert */

  /* Code blocks */
  --code-bg: #0d0d14;
  --code-border: #2a2a3a;
}
```

### Light Mode

```css
:root[data-theme="light"] {
  /* Background layers */
  --bg-deep: #fafafa;
  --bg-surface: #ffffff;
  --bg-elevated: #f0f0f5;

  /* Synthwave accent colors (adjusted for light bg) */
  --neon-cyan: #008b8b;
  --neon-magenta: #c71585;
  --neon-pink: #db2777;
  --neon-purple: #7c3aed;
  --neon-yellow: #b8860b;

  /* Text colors */
  --text-primary: #1a1a2e;
  --text-secondary: #4a4a5a;
  --text-dim: #8a8a9a;

  /* Code blocks */
  --code-bg: #f5f5fa;
  --code-border: #e0e0e8;
}
```

## Directory Structure

```
tools/sphinx-synthwave-theme/
├── pyproject.toml
├── README.md
├── LICENSE
└── sphinx_synthwave_theme/
    ├── __init__.py
    ├── theme.conf
    ├── static/
    │   ├── css/
    │   │   ├── synthwave.css          # Main stylesheet
    │   │   ├── responsive.css         # Breakpoints
    │   │   ├── components.css         # UI components
    │   │   ├── code-blocks.css        # Syntax highlighting
    │   │   └── theme-toggle.css       # Dark/light switch
    │   ├── js/
    │   │   ├── theme-toggle.js        # Theme switcher logic
    │   │   └── synthwave.js           # Additional interactivity
    │   └── fonts/                      # Optional custom fonts
    │       └── .gitkeep
    └── templates/
        ├── layout.html                 # Base template
        ├── page.html                   # Content pages
        ├── search.html                 # Search page
        └── components/
            ├── sidebar.html            # Navigation sidebar
            ├── toc.html                # Table of contents
            ├── header.html             # Top header
            ├── footer.html             # Page footer
            └── theme-toggle.html       # Theme switcher widget
```

## Theme Features

### 1. Responsive Breakpoints

```css
/* Mobile first */
@media (min-width: 640px)  { /* sm */ }
@media (min-width: 768px)  { /* md */ }
@media (min-width: 1024px) { /* lg */ }
@media (min-width: 1280px) { /* xl */ }
```

### 2. Theme Toggle

- SVG icons for sun/moon/system
- Stores preference in `localStorage`
- Respects `prefers-color-scheme` media query
- Smooth CSS transitions between themes

### 3. Typography

- Clean, readable body text
- Monospace for code with synthwave colors
- Proper heading hierarchy with neon accents

### 4. Navigation

- Collapsible sidebar on mobile
- Sticky header with breadcrumbs
- Smooth scroll with offset for fixed header
- Active section highlighting in TOC

### 5. Code Blocks

- Custom Pygments style matching synthwave palette
- Copy button integration (sphinx_copybutton)
- Language badge in corner
- Line numbers option

### 6. Special Components

- Admonitions with neon borders (note, warning, danger, tip)
- Cards with hover glow effects (sphinx_design)
- Tables with alternating row colors
- Blockquotes with left border accent

## Implementation Steps

### Phase 1: Setup & Structure

1. Create `tools/sphinx-synthwave-theme/` directory
2. Create `pyproject.toml` with proper metadata
3. Create `__init__.py` with theme registration
4. Create `theme.conf` with theme configuration
5. Update `scripts/neonsignal-sphinx-setup.sh` to install the theme and create a default `conf.py` when missing (see **Sphinx Setup Script Edits**).

### Phase 2: Base Templates

1. Create `layout.html` extending Sphinx basic theme
2. Add `<meta>` tags for theme color and viewport
3. Implement header with logo and theme toggle
4. Create responsive sidebar template
5. Add footer with project info

### Phase 3: CSS Implementation

1. CSS custom properties for theming
2. Base reset and typography
3. Layout grid (sidebar + content)
4. Responsive breakpoints
5. Theme toggle animations

### Phase 4: JavaScript

1. Theme detection and storage
2. Toggle button functionality
3. Sidebar mobile menu
4. Smooth scroll behavior

### Phase 5: Code Styling

1. Custom Pygments style class
2. Code block container styling
3. Integration with sphinx_copybutton
4. Inline code styling

### Phase 6: Components

1. Admonition boxes
2. Tables
3. Definition lists
4. Task lists (myst)
5. sphinx_design cards/grids

### Phase 7: Polish

1. Focus states for accessibility
2. Print stylesheet
3. Reduced motion support
4. High contrast mode support

## Sphinx Setup Script Edits

Keep the setup script in sync with the theme so a fresh repo bootstraps correctly:

- Ensure `docs/source/conf.py` exists; create a minimal file if missing.
- Set `html_theme = "sphinx_synthwave_theme"` and keep `templates_path`/`html_static_path` aligned with theme assets.
- Install the local theme package from `tools/sphinx-synthwave-theme/`.
- Preserve existing content; only write missing files.
- Add clear logging for environment setup, theme install, and file creation.

## pyproject.toml

```toml
[build-system]
requires = ["hatchling"]
build-backend = "hatchling.build"

[project]
name = "sphinx-synthwave-theme"
version = "0.1.0"
description = "Synthwave-inspired Sphinx theme with dark/light modes"
readme = "README.md"
license = "MIT"
authors = [
    { name = "Simone Del Popolo", email = "simonedelpopolo@outlook.com" }
]
keywords = ["sphinx", "theme", "synthwave", "documentation"]
classifiers = [
    "Development Status :: 4 - Beta",
    "Framework :: Sphinx",
    "Framework :: Sphinx :: Theme",
    "License :: OSI Approved :: MIT License",
    "Programming Language :: Python :: 3",
    "Topic :: Documentation :: Sphinx",
]
requires-python = ">=3.10"
dependencies = [
    "sphinx>=7.0",
]

[project.entry-points."sphinx.html_themes"]
sphinx_synthwave_theme = "sphinx_synthwave_theme"

[tool.hatch.build.targets.wheel]
packages = ["sphinx_synthwave_theme"]
```

## Integration with NeonSignal Docs

After theme is created, update `docs/source/conf.py`:

```python
import sphinx_synthwave_theme

html_theme = "sphinx_synthwave_theme"
html_theme_path = [sphinx_synthwave_theme.get_html_theme_path()]

html_theme_options = {
    "light_logo": "logo-light.svg",
    "dark_logo": "logo-dark.svg",
    "sidebar_hide_name": False,
    "navigation_with_keys": True,
}
```

And update `docs/requirements.txt`:

```
-e ../tools/sphinx-synthwave-theme
```

## Backup Location

Before removing Furo:
```
docs.bak/           # Full backup of docs directory
```

## Files to Modify

| File | Action |
|------|--------|
| `docs/source/conf.py` | Change theme to sphinx_synthwave_theme |
| `docs/requirements.txt` | Remove furo, add local theme |
| `docs/source/_static/` | Add logo assets |

## Dependencies to Remove

- `furo`
- `sphinx-basic-ng` (furo dependency)
- `accessible-pygments` (if not needed)

## Timeline

1. **Phase 1-2**: Theme scaffolding and templates
2. **Phase 3-4**: CSS and JavaScript
3. **Phase 5-6**: Code styling and components
4. **Phase 7**: Polish and testing
