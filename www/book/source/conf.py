from __future__ import annotations

import sys
from pathlib import Path

# Add theme to path for local development
theme_path = Path(__file__).parent.parent.parent / "themes" / "sphinx-synthwave-theme"
sys.path.insert(0, str(theme_path))

project = "neonsignal"
author = "Simone Del Popolo"
release = "0.1.0"

extensions = [
    "myst_parser",
    "sphinx_copybutton",
    "sphinx_design",
]

# Use synthwave theme
html_theme = "sphinx_synthwave_theme"
html_theme_path = [str(theme_path)]

html_theme_options = {
    "sidebar_hide_name": False,
    "navigation_with_keys": True,
    "default_theme": "system",
    "show_toc_level": 2,
}

templates_path = ["_templates"]
html_static_path = ["_static"]
html_sidebars = {
    "**": ["globaltoc.html", "relations.html", "searchbox.html"],
}

source_suffix = {
    ".md": "markdown",
}

exclude_patterns = ["**/*.rst"]

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "tasklist",
]
