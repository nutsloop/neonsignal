# Sphinx Synthwave Theme

A synthwave-inspired Sphinx theme with responsive design and dark/light/system theme support.

## Features

- **Synthwave Aesthetic** - Neon colors, glowing accents, retro-futuristic vibes
- **Responsive Design** - Mobile-first, works on all screen sizes
- **Theme Modes** - Dark, Light, and System preference detection
- **Modern Sphinx** - Compatible with Sphinx 7.x and 8.x
- **Extension Support** - Works with myst_parser, sphinx_copybutton, sphinx_design

## Installation

### From Local Directory

```bash
pip install -e /path/to/sphinx-synthwave-theme
```

### In requirements.txt

```
-e ../tools/sphinx-synthwave-theme
```

## Usage

In your `conf.py`:

```python
html_theme = "sphinx_synthwave_theme"

html_theme_options = {
    "light_logo": "logo-light.svg",
    "dark_logo": "logo-dark.svg",
    "sidebar_hide_name": False,
    "navigation_with_keys": True,
}
```

## Theme Options

| Option | Default | Description |
|--------|---------|-------------|
| `light_logo` | `None` | Logo for light theme |
| `dark_logo` | `None` | Logo for dark theme |
| `sidebar_hide_name` | `False` | Hide project name in sidebar |
| `navigation_with_keys` | `True` | Enable keyboard navigation |
| `default_theme` | `"system"` | Default theme: `"dark"`, `"light"`, or `"system"` |

## Restore Light/System Theme Support

The theme is currently locked to dark-only in `layout.html`. To restore dark/light/system
support, revert these changes:

1) In `tools/sphinx-synthwave-theme/sphinx_synthwave_theme/layout.html`:

- Replace:
  - `<meta name="color-scheme" content="dark">`
  - `<meta name="theme-color" content="#0a0a0f">`

  With:
  - `<meta name="color-scheme" content="dark light">`
  - `<meta name="theme-color" content="#0a0a0f" media="(prefers-color-scheme: dark)">`
  - `<meta name="theme-color" content="#fafafa" media="(prefers-color-scheme: light)">`

- Re-add the theme toggle script:
  - `<script src="{{ pathto('_static/js/theme-toggle.js', 1) }}"></script>`

- Re-add the theme toggle button include in the header:
  - `{% include "theme-toggle.html" %}`

2) Rebuild the docs so the updated theme assets are picked up.

## Color Palette

### Dark Mode
- Background: Deep purple-black (`#0a0a0f`)
- Cyan accent: `#00ffff`
- Magenta accent: `#ff00ff`
- Pink highlight: `#ff1493`

### Light Mode
- Background: Clean white (`#fafafa`)
- Adjusted accents for readability

## License

MIT License - see LICENSE file.
