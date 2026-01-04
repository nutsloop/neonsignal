"""Sphinx Synthwave Theme - A neon-inspired documentation theme."""

from __future__ import annotations

from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from sphinx.application import Sphinx

__version__ = "0.1.0"


def get_html_theme_path() -> str:
    """Return the path to the theme directory."""
    return str(Path(__file__).parent)


def setup(app: Sphinx) -> dict:
    """Register the theme with Sphinx."""
    app.add_html_theme("sphinx_synthwave_theme", str(Path(__file__).parent))

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
