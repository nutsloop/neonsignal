# SimoneDelPopolo Visual Performance Banner Plan

## Context snapshot
- Project: `simonedelpopolo/` is static HTML + shared `css/wasteland.css`, no JS files or runtime bundle.
- Heavy animations come from CSS (e.g., `grid-floor`, `scanlines::after`, `h1` glow/glitch, `entry::before` pulses).
- No existing visual toggle in this project; reference behavior exists in `neonsignaljsx/components/auth/visual_toggle.tsx` via localStorage + `data-visual` on `body`.

## Goal
Detect when the browser is struggling with the animation load and show a small banner offering to reduce visuals. Selecting “reduce” should disable the most expensive effects and persist preference.

## Proposed approach
1) **Add a tiny runtime plugin**
   - New JS file (e.g., `simonedelpopolo/scripts/visual-performance.js`) injected by all HTML pages.
   - Self-contained: no build step, minimal DOM changes (creates banner dynamically).

2) **Struggle detection heuristic (make reasonable)**
   - Use `requestAnimationFrame` to sample frame time deltas for ~3–5 seconds.
   - Count frames with `delta > 50ms` (i.e., <20 FPS) and/or compute average FPS.
   - Trigger banner if `jankCount >= 8` or `avgFps < 45`.
   - Optional boost: `PerformanceObserver` for `longtask` entries; if total longtask time > 300ms in the window, treat as struggling.
   - Respect `prefers-reduced-motion`: if enabled, skip detection and force low mode immediately.

3) **Persistent visual mode**
   - Use the same key as `visual_toggle.tsx` (`neonsignal_visual_mode`) for consistency.
   - Values: `full` | `low`. Read at startup; if `low`, skip detection and apply low mode.

4) **Low-visual CSS strategy**
   - Add a small CSS section or override file that activates when `body[data-visual='low']`.
   - Target the most expensive effects instead of nuking all styles:
     - Disable animations on `h1`, `h1::before`, `h1::after`, `.grid-floor`, `.scanlines::after`, `.tagline`, `.entry::before`, `.blink`, `.card`, `.entry`.
     - Hide `.scanlines` and `.noise` overlays or reduce opacity to near-zero.
     - Remove animated transforms on `.entry:hover` and other transitions if needed.
   - Keep typography/spacing intact for visual continuity.

5) **Banner UX**
   - Inject a small fixed banner (bottom-right or bottom-center) with short text: “Performance looks strained — reduce visuals?”
   - Buttons: “Reduce” (apply low mode + persist), “Dismiss” (hide banner for session).
   - Keep banner tiny and low-contrast to avoid visual noise; add `aria-live="polite"`.

6) **Integration steps (no implementation now)**
   - Add the JS file and include via `<script src="/scripts/visual-performance.js" defer></script>` in each HTML file.
   - Add CSS overrides in `css/wasteland.css` or a new `css/visual-mode.css` file (and link it).
   - Consider a small `noscript` note optional (not required).

## Edge cases / assumptions
- If the browser blocks `localStorage`, plugin should default to `full` and continue detection without throwing.
- Pages are static, so script inclusion must be repeated or templated (manual editing is acceptable if no templating is present).
- The user might switch to low mode from another site using the same key; matching `neonsignal_visual_mode` is intentional.

## Quick test ideas
- Use DevTools CPU throttling (4× or 6×) to simulate jank; verify banner appears.
- Verify that “Reduce” disables animations and persists across reloads.
- Verify “Dismiss” only hides the banner for the current session (no storage change).

## Implementation notes (applied)
- Added `simonedelpopolo/scripts/visual-performance.js` with rAF sampling, optional long-task detection, banner UI, and `data-visual` handling.
- Added banner styling + low-visual overrides in `simonedelpopolo/css/wasteland.css`.
- Injected `<script src="/scripts/visual-performance.js" defer></script>` into all SimoneDelPopolo HTML pages.

## How to use
- Load any page; if the browser struggles, a small banner appears after ~4s.
- Click “Reduce” to persist low-visual mode (`neonsignal_visual_mode=low`).
- Click “Dismiss” to hide the banner for this session only.
- Remove low mode by clearing `neonsignal_visual_mode` in localStorage or setting it to `full`.
