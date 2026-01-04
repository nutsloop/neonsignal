/**
 * Sphinx Synthwave Theme - Theme Toggle
 * Handles dark/light/system theme switching with localStorage persistence
 */

(function() {
  'use strict';

  const STORAGE_KEY = 'synthwave-theme';
  const THEMES = ['dark', 'light', 'system'];

  /**
   * Get the system's preferred color scheme
   */
  function getSystemTheme() {
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: light)').matches) {
      return 'light';
    }
    return 'dark';
  }

  /**
   * Get the effective theme (resolves 'system' to actual theme)
   */
  function getEffectiveTheme(theme) {
    return theme === 'system' ? getSystemTheme() : theme;
  }

  /**
   * Get stored theme preference
   */
  function getStoredTheme() {
    try {
      const stored = localStorage.getItem(STORAGE_KEY);
      if (stored && THEMES.includes(stored)) {
        return stored;
      }
    } catch (e) {
      // localStorage not available
    }
    return 'system';
  }

  /**
   * Store theme preference
   */
  function setStoredTheme(theme) {
    try {
      localStorage.setItem(STORAGE_KEY, theme);
    } catch (e) {
      // localStorage not available
    }
  }

  /**
   * Apply theme to document
   */
  function applyTheme(theme) {
    const effective = getEffectiveTheme(theme);
    document.documentElement.setAttribute('data-theme', theme);

    // Update meta theme-color for mobile browsers
    const metaThemeColor = document.querySelector('meta[name="theme-color"]');
    if (metaThemeColor) {
      metaThemeColor.setAttribute('content', effective === 'dark' ? '#0a0a0f' : '#fafafa');
    }
  }

  /**
   * Cycle to next theme
   */
  function cycleTheme() {
    const current = getStoredTheme();
    const currentIndex = THEMES.indexOf(current);
    const nextIndex = (currentIndex + 1) % THEMES.length;
    const nextTheme = THEMES[nextIndex];

    setStoredTheme(nextTheme);
    applyTheme(nextTheme);

    return nextTheme;
  }

  /**
   * Initialize theme toggle button
   */
  function initToggle() {
    const toggle = document.getElementById('theme-toggle');
    if (!toggle) return;

    toggle.addEventListener('click', function() {
      const newTheme = cycleTheme();

      // Update button title
      const titles = {
        dark: 'Current: Dark theme. Click for Light.',
        light: 'Current: Light theme. Click for System.',
        system: 'Current: System theme. Click for Dark.'
      };
      toggle.setAttribute('title', titles[newTheme] || 'Toggle theme');
    });

    // Set initial title
    const current = getStoredTheme();
    const titles = {
      dark: 'Current: Dark theme. Click for Light.',
      light: 'Current: Light theme. Click for System.',
      system: 'Current: System theme. Click for Dark.'
    };
    toggle.setAttribute('title', titles[current] || 'Toggle theme');
  }

  /**
   * Listen for system theme changes
   */
  function watchSystemTheme() {
    if (!window.matchMedia) return;

    const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)');

    mediaQuery.addEventListener('change', function() {
      // Only update if using system theme
      if (getStoredTheme() === 'system') {
        applyTheme('system');
      }
    });
  }

  // Apply theme immediately to prevent flash
  (function() {
    const theme = getStoredTheme();
    applyTheme(theme);
  })();

  // Initialize when DOM is ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function() {
      initToggle();
      watchSystemTheme();
    });
  } else {
    initToggle();
    watchSystemTheme();
  }

})();
