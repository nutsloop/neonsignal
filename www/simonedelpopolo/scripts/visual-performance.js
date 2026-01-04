(() => {
  const STORAGE_KEY = 'neonsignal_visual_mode';
  const BANNER_DISMISSED_KEY = 'neonsignal_visual_banner_dismissed';
  const LOW_MODE = 'low';

  const safeStorage = (name) => {
    try {
      return window[name];
    } catch {
      return null;
    }
  };

  const storage = safeStorage('localStorage');
  const session = safeStorage('sessionStorage');
  let currentMode = 'full';
  let measuring = false;
  const deviceHints = (() => {
    const cores = Number.isFinite(navigator.hardwareConcurrency)
      ? navigator.hardwareConcurrency
      : null;
    const memory = Number.isFinite(navigator.deviceMemory)
      ? navigator.deviceMemory
      : null;
    const lowPower =
      (cores !== null && cores <= 4) ||
      (memory !== null && memory <= 4);
    return {
      cores,
      memory,
      lowPower,
      label: `HW ${cores ?? '??'}C ${memory ?? '??'}G`
    };
  })();

  const readStoredMode = () => {
    if (!storage) {
      return 'full';
    }
    try {
      return storage.getItem(STORAGE_KEY) === LOW_MODE ? LOW_MODE : 'full';
    } catch {
      return 'full';
    }
  };

  const writeStoredMode = (mode) => {
    if (!storage) {
      return;
    }
    try {
      storage.setItem(STORAGE_KEY, mode);
    } catch {
      return;
    }
  };

  const isDismissed = () => {
    if (!session) {
      return false;
    }
    try {
      return session.getItem(BANNER_DISMISSED_KEY) === '1';
    } catch {
      return false;
    }
  };

  const markDismissed = () => {
    if (!session) {
      return;
    }
    try {
      session.setItem(BANNER_DISMISSED_KEY, '1');
    } catch {
      return;
    }
  };

  const applyMode = (mode) => {
    if (!document.body) {
      return;
    }
    document.body.setAttribute('data-visual', mode);
    writeStoredMode(mode);
    currentMode = mode;
    updateStick(mode);
    updatePanel(mode);
  };

  const removeBanner = () => {
    const banner = document.getElementById('visual-banner');
    if (banner) {
      banner.remove();
    }
  };

  const ensurePerfStick = () => {
    let stick = document.getElementById('visual-perf-stick');
    if (stick) {
      return stick;
    }
    stick = document.createElement('div');
    stick.id = 'visual-perf-stick';
    stick.setAttribute('role', 'button');
    stick.setAttribute('tabindex', '0');
    stick.setAttribute('aria-label', 'Visual performance options');
    stick.style.cssText = [
      'position: fixed',
      'top: 0.75rem',
      'right: 0.75rem',
      'padding: 0.35rem 0.55rem',
      'border-radius: 999px',
      'border: 1px solid rgba(0, 255, 249, 0.35)',
      'background: rgba(5, 1, 13, 0.9)',
      'color: rgba(224, 224, 232, 0.75)',
      'font-family: \"Share Tech Mono\", monospace',
      'font-size: 0.6rem',
      'letter-spacing: 0.12em',
      'text-transform: uppercase',
      'z-index: 2001',
      'pointer-events: auto',
      'cursor: pointer',
      'user-select: none',
      'box-shadow: 0 0 12px rgba(0, 255, 249, 0.18)'
    ].join('; ');
    stick.textContent = `VIS FULL · ${deviceHints.label} · PERF …`;
    stick.addEventListener('click', () => {
      togglePanel();
    });
    stick.addEventListener('keydown', (event) => {
      if (event.key === 'Enter' || event.key === ' ') {
        event.preventDefault();
        togglePanel();
      }
    });
    document.body.appendChild(stick);
    return stick;
  };

  const updateStick = (mode, metrics) => {
    const stick = ensurePerfStick();
    const label = mode === LOW_MODE ? 'LOW' : 'FULL';
    if (!metrics) {
      stick.textContent =
        `VIS ${label} · ${deviceHints.label} · PERF ${mode === LOW_MODE ? 'OFF' : 'ON'}`;
      return;
    }
    stick.textContent =
      `VIS ${label} · ${deviceHints.label} · FPS ${metrics.fps} · J ${metrics.jank} · LT ${metrics.longTask}`;
  };

  const updatePanel = (mode) => {
    const toggle = document.getElementById('visual-mode-toggle');
    if (toggle) {
      toggle.checked = mode === LOW_MODE;
    }
    const status = document.getElementById('visual-mode-status');
    if (status) {
      status.textContent = mode === LOW_MODE ? 'Low visuals enabled' : 'Full visuals enabled';
    }
  };

  const togglePanel = () => {
    const existing = document.getElementById('visual-perf-panel');
    if (existing) {
      existing.remove();
      return;
    }

    const panel = document.createElement('div');
    panel.id = 'visual-perf-panel';
    panel.style.cssText = [
      'position: fixed',
      'top: 2.6rem',
      'right: 0.75rem',
      'min-width: 200px',
      'padding: 0.75rem 0.9rem',
      'border-radius: 10px',
      'border: 1px solid rgba(157, 78, 221, 0.45)',
      'background: rgba(10, 5, 21, 0.92)',
      'box-shadow: 0 0 18px rgba(157, 78, 221, 0.2)',
      'backdrop-filter: blur(6px)',
      'color: rgba(224, 224, 232, 0.85)',
      'font-family: \"Share Tech Mono\", monospace',
      'font-size: 0.65rem',
      'letter-spacing: 0.08em',
      'text-transform: uppercase',
      'z-index: 2001'
    ].join('; ');

    const title = document.createElement('div');
    title.textContent = 'Visual Controls';
    title.style.cssText = [
      'font-family: \"Orbitron\", sans-serif',
      'font-size: 0.65rem',
      'letter-spacing: 0.18em',
      'margin-bottom: 0.5rem',
      'color: rgba(0, 255, 249, 0.75)'
    ].join('; ');

    const label = document.createElement('label');
    label.style.cssText = [
      'display: flex',
      'align-items: center',
      'gap: 0.5rem',
      'cursor: pointer'
    ].join('; ');

    const toggle = document.createElement('input');
    toggle.type = 'checkbox';
    toggle.id = 'visual-mode-toggle';
    toggle.checked = currentMode === LOW_MODE;
    toggle.addEventListener('change', () => {
      applyMode(toggle.checked ? LOW_MODE : 'full');
      removeBanner();
      if (!toggle.checked && document.visibilityState !== 'hidden') {
        measurePerformance();
      }
    });

    const labelText = document.createElement('span');
    labelText.textContent = 'Low visuals';

    const status = document.createElement('div');
    status.id = 'visual-mode-status';
    status.style.cssText = [
      'margin-top: 0.45rem',
      'font-size: 0.55rem',
      'letter-spacing: 0.12em',
      'color: rgba(224, 224, 232, 0.6)'
    ].join('; ');

    label.appendChild(toggle);
    label.appendChild(labelText);
    panel.appendChild(title);
    panel.appendChild(label);
    panel.appendChild(status);
    document.body.appendChild(panel);
    updatePanel(currentMode);
  };

  const showBanner = () => {
    if (isDismissed() || readStoredMode() === LOW_MODE) {
      return;
    }
    if (document.getElementById('visual-banner')) {
      return;
    }

    const banner = document.createElement('div');
    banner.id = 'visual-banner';
    banner.className = 'visual-banner';
    banner.setAttribute('role', 'status');
    banner.setAttribute('aria-live', 'polite');

    const text = document.createElement('div');
    text.className = 'visual-banner__text';
    text.textContent = 'Performance looks strained. Reduce visuals?';

    const actions = document.createElement('div');
    actions.className = 'visual-banner__actions';

    const reduce = document.createElement('button');
    reduce.type = 'button';
    reduce.className = 'visual-banner__button visual-banner__button--primary';
    reduce.textContent = 'Reduce';
    reduce.addEventListener('click', () => {
      applyMode(LOW_MODE);
      removeBanner();
    });

    const dismiss = document.createElement('button');
    dismiss.type = 'button';
    dismiss.className = 'visual-banner__button';
    dismiss.textContent = 'Dismiss';
    dismiss.addEventListener('click', () => {
      markDismissed();
      removeBanner();
    });

    actions.appendChild(reduce);
    actions.appendChild(dismiss);
    banner.appendChild(text);
    banner.appendChild(actions);

    document.body.appendChild(banner);
  };

  const measurePerformance = () => {
    if (measuring) {
      return;
    }
    measuring = true;
    const sampleDurationMs = 4000;
    const jankThreshold = deviceHints.lowPower ? 6 : 8;
    const fpsThreshold = deviceHints.lowPower ? 50 : 45;
    const longTaskThreshold = deviceHints.lowPower ? 200 : 300;
    let last = performance.now();
    let start = last;
    let frames = 0;
    let jankCount = 0;
    let longTaskTotal = 0;
    let observer = null;
    ensurePerfStick();
    const stopMeasurement = () => {
      if (observer) {
        observer.disconnect();
        observer = null;
      }
      measuring = false;
    };

    if ('PerformanceObserver' in window) {
      try {
        observer = new PerformanceObserver((list) => {
          for (const entry of list.getEntries()) {
            longTaskTotal += entry.duration;
          }
        });
        observer.observe({ type: 'longtask', buffered: true });
      } catch {
        observer = null;
      }
    }

    const tick = (now) => {
      if (currentMode === LOW_MODE) {
        stopMeasurement();
        updateStick(LOW_MODE);
        return;
      }

      const delta = now - last;
      if (delta > 50) {
        jankCount += 1;
      }
      frames += 1;
      last = now;
      if (frames % 10 === 0) {
        const elapsed = now - start;
        const avgFps = elapsed > 0 ? frames / (elapsed / 1000) : 0;
        const longTaskLabel = observer ? `${Math.round(longTaskTotal)}ms` : 'n/a';
        updateStick(currentMode, {
          fps: Math.round(avgFps),
          jank: jankCount,
          longTask: longTaskLabel
        });
      }

      if (now - start < sampleDurationMs) {
        requestAnimationFrame(tick);
        return;
      }

      stopMeasurement();

      const elapsed = now - start;
      const avgFps = frames / (elapsed / 1000);
      const longTaskLabel = observer ? `${Math.round(longTaskTotal)}ms` : 'n/a';
      updateStick(currentMode, {
        fps: Math.round(avgFps),
        jank: jankCount,
        longTask: longTaskLabel
      });
      if (jankCount >= jankThreshold || avgFps < fpsThreshold || longTaskTotal > longTaskThreshold) {
        showBanner();
      }
    };

    requestAnimationFrame(tick);
  };

  const start = () => {
    if (!document.body) {
      return;
    }

    const prefersReduced =
      typeof window.matchMedia === 'function' &&
      window.matchMedia('(prefers-reduced-motion: reduce)').matches;

    const storedMode = readStoredMode();
    if (storedMode === LOW_MODE || prefersReduced) {
      applyMode(LOW_MODE);
      return;
    }

    applyMode('full');

    if (document.visibilityState === 'hidden') {
      return;
    }

    measurePerformance();
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', start, { once: true });
  } else {
    start();
  }
})();
