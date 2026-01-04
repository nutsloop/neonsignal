/**
 * Sphinx Synthwave Theme - Main JavaScript
 * Handles sidebar, navigation, and other interactive features
 */

(function() {
  'use strict';

  /**
   * Sidebar toggle for mobile
   */
  function initSidebar() {
    const toggle = document.querySelector('.sidebar-toggle');
    const sidebar = document.querySelector('.synthwave-sidebar');

    if (!toggle || !sidebar) return;

    // Create overlay element
    const overlay = document.createElement('div');
    overlay.className = 'sidebar-overlay';
    document.body.appendChild(overlay);

    function openSidebar() {
      sidebar.classList.add('open');
      overlay.classList.add('active');
      toggle.setAttribute('aria-expanded', 'true');
      document.body.style.overflow = 'hidden';
    }

    function closeSidebar() {
      sidebar.classList.remove('open');
      overlay.classList.remove('active');
      toggle.setAttribute('aria-expanded', 'false');
      document.body.style.overflow = '';
    }

    toggle.addEventListener('click', function() {
      if (sidebar.classList.contains('open')) {
        closeSidebar();
      } else {
        openSidebar();
      }
    });

    overlay.addEventListener('click', closeSidebar);

    // Close on escape key
    document.addEventListener('keydown', function(e) {
      if (e.key === 'Escape' && sidebar.classList.contains('open')) {
        closeSidebar();
      }
    });

    // Close when clicking a link (mobile)
    sidebar.querySelectorAll('a').forEach(function(link) {
      link.addEventListener('click', function() {
        if (window.innerWidth < 768) {
          closeSidebar();
        }
      });
    });
  }

  /**
   * Highlight current section in TOC
   */
  function initTocHighlight() {
    const toc = document.querySelector('.synthwave-toc');
    if (!toc) return;

    const links = toc.querySelectorAll('a');
    const headings = [];

    links.forEach(function(link) {
      const href = link.getAttribute('href');
      if (href && href.startsWith('#')) {
        const id = href.slice(1);
        const heading = document.getElementById(id);
        if (heading) {
          headings.push({ link: link, heading: heading });
        }
      }
    });

    if (headings.length === 0) return;

    function updateHighlight() {
      const scrollPos = window.scrollY + 100; // Offset for header

      let current = null;
      for (let i = headings.length - 1; i >= 0; i--) {
        if (headings[i].heading.offsetTop <= scrollPos) {
          current = headings[i];
          break;
        }
      }

      links.forEach(function(link) {
        link.classList.remove('active');
      });

      if (current) {
        current.link.classList.add('active');
      }
    }

    // Throttle scroll events
    let ticking = false;
    window.addEventListener('scroll', function() {
      if (!ticking) {
        requestAnimationFrame(function() {
          updateHighlight();
          ticking = false;
        });
        ticking = true;
      }
    });

    // Initial highlight
    updateHighlight();
  }

  /**
   * Add language labels to code blocks
   */
  function initCodeBlocks() {
    document.querySelectorAll('div.highlight').forEach(function(block) {
      // Try to find language from class
      const classes = block.className.split(' ');
      for (let i = 0; i < classes.length; i++) {
        const cls = classes[i];
        if (cls.startsWith('highlight-') && cls !== 'highlight') {
          const lang = cls.replace('highlight-', '');
          if (lang && lang !== 'default') {
            block.setAttribute('data-lang', lang);
          }
          break;
        }
      }
    });
  }

  /**
   * Smooth scroll for anchor links
   */
  function initSmoothScroll() {
    document.querySelectorAll('a[href^="#"]').forEach(function(link) {
      link.addEventListener('click', function(e) {
        const href = this.getAttribute('href');
        if (href === '#') return;

        const target = document.querySelector(href);
        if (target) {
          e.preventDefault();
          const headerHeight = document.querySelector('.synthwave-header')?.offsetHeight || 0;
          const targetPos = target.offsetTop - headerHeight - 20;

          window.scrollTo({
            top: targetPos,
            behavior: 'smooth'
          });

          // Update URL without scrolling
          history.pushState(null, null, href);
        }
      });
    });
  }

  /**
   * Keyboard navigation
   */
  function initKeyboardNav() {
    document.addEventListener('keydown', function(e) {
      // Skip if typing in input
      if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') {
        return;
      }

      // Left arrow - previous page
      if (e.key === 'ArrowLeft' && !e.ctrlKey && !e.metaKey && !e.shiftKey) {
        const prev = document.querySelector('.nav-prev');
        if (prev) {
          prev.click();
        }
      }

      // Right arrow - next page
      if (e.key === 'ArrowRight' && !e.ctrlKey && !e.metaKey && !e.shiftKey) {
        const next = document.querySelector('.nav-next');
        if (next) {
          next.click();
        }
      }

      // / - focus search (if exists)
      if (e.key === '/' && !e.ctrlKey && !e.metaKey) {
        const search = document.querySelector('input[type="search"], input[name="q"]');
        if (search) {
          e.preventDefault();
          search.focus();
        }
      }
    });
  }

  /**
   * Mark current page in sidebar navigation
   */
  function initCurrentPage() {
    const currentPath = window.location.pathname;
    const sidebarLinks = document.querySelectorAll('.sidebar-nav a');

    sidebarLinks.forEach(function(link) {
      const linkPath = new URL(link.href).pathname;
      if (linkPath === currentPath) {
        link.classList.add('current');
      }
    });
  }

  // Initialize all features when DOM is ready
  function init() {
    initSidebar();
    initTocHighlight();
    initCodeBlocks();
    initSmoothScroll();
    initKeyboardNav();
    initCurrentPage();
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }

})();
