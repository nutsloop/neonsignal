# Plan: NeonJSX SPA Routing Detection

## Problem Statement

Currently, the SPA (Single Page Application) routing logic in `handle_io_.c++` (lines 424-439) serves `index.html` for **all** 404 HTML requests across all virtual hosts. This allows NeonJSX apps to handle client-side routing.

However, not all virtual hosts are NeonJSX apps:
- **NeonJSX apps**: Should receive `index.html` shell for client-side routing (e.g., `/dashboard` → `index.html` with `window.__NEON_PATH`)
- **Static sites**: Should return real 404 errors (e.g., `simonedelpopolo.host/nonexistent.html` → 404)

**Current behavior**: ALL virtual hosts get SPA routing, causing static sites to never return true 404s for HTML paths.

---

## Proposed Solution: Route Manifest File

Place a `.neonjsx` file in the root of any virtual host that should have SPA routing. The file contains a list of valid page component routes, enabling NeonSignal to:

1. **Known route** (in manifest) → Serve `index.html` with **status 200** + `window.__NEON_PATH`
2. **Unknown route** (not in manifest) → Serve `index.html` with **status 404** + `window.__NEON_STATUS=404`
3. **No `.neonjsx` file** → Real 404 (static site behavior)

This gives NeonJSX proper context to render the correct component or a "not found" page.

### File Format

```
public/neonsignal.nutsloop.host/.neonjsx
```

```
# NeonJSX Page Components
# One route per line, supports patterns

/
/data
/data.html
/auth
/auth.html
/dashboard
/settings
/settings/*
```

**Format rules:**
- One route per line
- Lines starting with `#` are comments
- Empty lines are ignored
- `*` suffix matches any sub-path (e.g., `/settings/*` matches `/settings/profile`)
- Exact match by default (e.g., `/data` only matches `/data`, not `/data/foo`)

### Directory Structure

```
public/
├── neonsignal.nutsloop.host/
│   ├── .neonjsx              ← Route manifest (list of page components)
│   ├── index.html
│   └── ...
├── simonedelpopolo.host/     ← No .neonjsx → Static site (real 404s)
│   ├── index.html
│   └── css/
└── _default/
    └── index.html
```

---

## Implementation Plan

### Step 1: Create NeonJSX config struct

**File:** `include/neonsignal/vhost.h++`

```cpp
struct NeonJSXConfig {
  bool enabled{false};
  std::vector<std::string> routes;        // Exact match routes
  std::vector<std::string> wildcard_routes; // Routes ending with /*

  // Check if a path matches a known route
  [[nodiscard]] bool matches_route(std::string_view path) const;
};

struct VirtualHost {
  std::string domain;
  std::filesystem::path document_root;
  bool is_wildcard{false};
  NeonJSXConfig neonjsx;  // NEW: NeonJSX configuration
};
```

### Step 2: Parse `.neonjsx` file during vhost discovery

**File:** `src/neonsignal/vhost/resolve.c++`

Add a helper to parse the manifest:

```cpp
NeonJSXConfig parse_neonjsx_config(const std::filesystem::path &config_path) {
  NeonJSXConfig config;

  std::ifstream file(config_path);
  if (!file.is_open()) {
    return config;  // File doesn't exist or can't be read
  }

  config.enabled = true;
  std::string line;
  while (std::getline(file, line)) {
    // Trim whitespace
    auto start = line.find_first_not_of(" \t");
    if (start == std::string::npos) continue;  // Empty line
    auto end = line.find_last_not_of(" \t\r\n");
    line = line.substr(start, end - start + 1);

    // Skip comments
    if (line.empty() || line[0] == '#') continue;

    // Wildcard routes end with /*
    if (line.size() > 2 && line.substr(line.size() - 2) == "/*") {
      config.wildcard_routes.push_back(line.substr(0, line.size() - 2));
    } else {
      config.routes.push_back(line);
    }
  }

  return config;
}
```

In `VHostResolver::refresh()`:

```cpp
vhost.neonjsx = parse_neonjsx_config(entry.path() / ".neonjsx");
```

### Step 3: Implement route matching

**File:** `src/neonsignal/vhost/resolve.c++`

```cpp
bool NeonJSXConfig::matches_route(std::string_view path) const {
  if (!enabled) return false;

  // Check exact matches
  for (const auto &route : routes) {
    if (path == route) return true;
  }

  // Check wildcard matches (prefix match)
  for (const auto &prefix : wildcard_routes) {
    if (path.starts_with(prefix)) return true;
  }

  return false;
}
```

### Step 4: Add query methods to VHostResolver

**File:** `include/neonsignal/vhost.h++`

```cpp
class VHostResolver {
public:
  // ... existing methods ...

  // Check if domain has NeonJSX SPA routing enabled
  [[nodiscard]] bool is_neonjsx(std::string_view authority) const;

  // Check if path is a known NeonJSX route for this domain
  [[nodiscard]] bool is_neonjsx_route(std::string_view authority,
                                       std::string_view path) const;
};
```

**File:** `src/neonsignal/vhost/resolve.c++`

```cpp
bool VHostResolver::is_neonjsx(std::string_view authority) const {
  std::shared_lock lock(mutex_);
  const std::string domain = normalize_authority(authority);

  if (const auto it = exact_hosts_.find(domain); it != exact_hosts_.end()) {
    return it->second.neonjsx.enabled;
  }
  if (has_default_) {
    return exact_hosts_.at("_default").neonjsx.enabled;
  }
  return false;
}

bool VHostResolver::is_neonjsx_route(std::string_view authority,
                                      std::string_view path) const {
  std::shared_lock lock(mutex_);
  const std::string domain = normalize_authority(authority);

  if (const auto it = exact_hosts_.find(domain); it != exact_hosts_.end()) {
    return it->second.neonjsx.matches_route(path);
  }
  if (has_default_) {
    return exact_hosts_.at("_default").neonjsx.matches_route(path);
  }
  return false;
}
```

### Step 5: Update SPA routing logic

**File:** `src/neonsignal/http2_listener/handle_io_.c++`

Replace the hardcoded `/data` and `/auth` checks and the generic 404 handler:

```cpp
// Remove hardcoded /data and /auth handlers (lines 406-423)
// Replace with unified NeonJSX routing:

if (res.status == 404 && is_html && vhost_resolver_.is_neonjsx(authority)) {
  auto shell = load_shell();
  if (shell.status == 200) {
    res.content_type = shell.content_type;

    // Check if this is a known route
    bool is_known_route = vhost_resolver_.is_neonjsx_route(authority, path);

    // Known route = 200, Unknown route = 404
    res.status = is_known_route ? 200 : 404;

    std::string body_str(shell.body.begin(), shell.body.end());
    body_str += "<script>";
    if (!is_known_route) {
      body_str += "window.__NEON_STATUS=404;";
    }
    body_str += "window.__NEON_PATH=\"";
    body_str += path;
    body_str += "\";</script>";
    res.body.assign(body_str.begin(), body_str.end());
  }
}
// If not NeonJSX, res stays 404 and returns default 404 response
```

---

## Files to Modify

| File | Change |
|------|--------|
| `include/neonsignal/vhost.h++` | Add `NeonJSXConfig` struct, add query methods |
| `src/neonsignal/vhost/resolve.c++` | Parse `.neonjsx`, implement route matching |
| `src/neonsignal/http2_listener/handle_io_.c++` | Unified SPA routing with route awareness |

---

## Example `.neonjsx` File

```bash
# public/neonsignal.nutsloop.host/.neonjsx
```

```
# NeonSignal Dashboard Routes
# These are the page components NeonJSX can render

# Root
/

# Dashboard pages
/data
/data.html
/auth
/auth.html

# Future routes
/settings
/settings/*
/profile
```

---

## Routing Behavior Matrix

| Domain | Path | `.neonjsx` | In Manifest? | HTTP Status | Response |
|--------|------|------------|--------------|-------------|----------|
| neonsignal.nutsloop.host | `/data` | Yes | Yes | 200 | `index.html` + `__NEON_PATH="/data"` |
| neonsignal.nutsloop.host | `/unknown` | Yes | No | 404 | `index.html` + `__NEON_STATUS=404` |
| simonedelpopolo.host | `/anything` | No | N/A | 404 | Real 404 (no shell) |

---

## Testing

1. **Create manifest for NeonJSX vhost:**
   ```bash
   cat > public/neonsignal.nutsloop.host/.neonjsx << 'EOF'
   # NeonJSX routes
   /
   /data
   /data.html
   /auth
   /auth.html
   EOF
   ```

2. **Known route returns 200:**
   ```bash
   curl -k -I https://neonsignal.nutsloop.host/data
   # HTTP/2 200
   ```

3. **Unknown route returns 404 with shell:**
   ```bash
   curl -k -I https://neonsignal.nutsloop.host/nonexistent
   # HTTP/2 404
   # Body contains index.html + __NEON_STATUS=404
   ```

4. **Static site returns real 404:**
   ```bash
   curl -k -I https://simonedelpopolo.host/nonexistent.html
   # HTTP/2 404 (no index.html shell)
   ```

5. **Verify logging:**
   ```
   vhost: neonsignal.nutsloop.host [neonjsx: 5 routes]
   vhost: simonedelpopolo.host
   ```

---

## Future Considerations

- **Runtime refresh:** Signal handler (SIGHUP) to reload `.neonjsx` files without restart
- **Extended config:** JSON format for additional options (cache headers, excluded paths, etc.)
- **VHScript integration:** `<spa routes="/,/data,/auth" />` directive
- **Pattern matching:** Regex routes like `/user/:id` for dynamic segments
