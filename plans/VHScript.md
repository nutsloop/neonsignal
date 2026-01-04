# VHScript: Virtual Host Configuration Language

## Overview

Design a lightweight configuration language for declaring virtual hosts in NeonSignal. VHScript provides explicit control over virtual host behavior while maintaining backward compatibility with auto-discovery.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        Configuration Priority                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   1. VHScript declarations (config/vhosts.vhs)     ← Highest priority       │
│   2. Auto-discovered directories (public/<domain>)                          │
│   3. _default fallback                              ← Lowest priority       │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Design Goals

1. **Familiar syntax** - XML-inspired tags with attributes (domain = root)
2. **Minimal boilerplate** - Sensible defaults, explicit overrides
3. **Single-file or split** - One `vhosts.vhs` or per-domain files
4. **Runtime reload** - SIGHUP to reload without restart
5. **Validation** - Syntax checking at startup with clear error messages (including path existence)

---

## Syntax Design (XML-style tags)

VHScript uses domain-named tags with attributes. Comments are `--` (Lua/SQL style). The opening tag binds the domain to an absolute root path; nested tags configure SPA, headers, and redirects. Paths are validated during parse.

```text
-- config/vhosts.vhs
-- VHScript v1 - NeonSignal Virtual Host Configuration

<defaults>
  ssl_min_version="TLS1.2"
  index="[index.html,index.htm]"
</defaults>

<simonedelpopolo.host=/home/core/code/neonsignal/public/simonedelpopolo.host>
  cert="/home/core/code/neonsignal/certs/simonedelpopolo.host"

  <spa enabled="true"
       routes="[ /, /app, /dashboard, /settings ]"
       exclude="[ /api, /assets ]" />

  <headers>
    <header name="X-Frame-Options" value="DENY" />
    <header name="X-Content-Type-Options" value="nosniff" />
    <header name="Strict-Transport-Security"
            value="max-age=31536000; includeSubDomains" />
  </headers>

  <redirect from="/old-path" to="/new-path" code="301" />
  <redirect from="/blog/*" to="https://blog.simonedelpopolo.host$1" code="302" />
</simonedelpopolo.host>

<neonsignal.nutsloop.host=/home/core/code/neonsignal/public/neonsignal.nutsloop.host>
  cert="/home/core/code/neonsignal/certs/neonsignal.nutsloop.host"
  <spa enabled="true"
       routes="[ /, /auth, /admin, /dashboard ]"
       exclude="[ /api, /events, /sse ]" />
</neonsignal.nutsloop.host>

-- Wildcard vhost
<*.nutsloop.host=/home/core/code/neonsignal/public/_wildcard_nutsloop>
  cert="/home/core/code/neonsignal/certs/*.nutsloop.host"
</*.nutsloop.host>

-- IP-based default (no SNI / unknown domains)
<_default=/home/core/code/neonsignal/public/_default>
  cert="/home/core/code/neonsignal/certs/_default"
</_default>

-- Aliases (multiple domains → same config)
<alias from="www.simonedelpopolo.host" to="simonedelpopolo.host" />
<alias from="simone.host" to="simonedelpopolo.host" />
```

### Syntax Highlights

- **`--` comments** - Line comments (SQL/Lua style)
- **Domain tags** - `<domain=/abs/root>` with matching `</domain>` closing tag
- **Attributes** - `key="value"` pairs inside tags; self-closing tags end with `/>`
- **Nested blocks** - `<spa .../>`, `<headers> <header .../> </headers>`, `<redirect .../>`
- **Path validation** - Parser verifies referenced paths exist at parse time

---

## Syntax Specification

```text
VHSCRIPT     := (COMMENT | DEFAULTS | VHOST | ALIAS | BLANK)*

COMMENT      := '#' TEXT NEWLINE
BLANK        := NEWLINE

DEFAULTS     := '<defaults>' PROP_LINE* '</defaults>'

VHOST        := '<' DOMAIN '=' PATH '>' VHOST_BODY '</' DOMAIN '>'
VHOST_BODY   := PROP_LINE* (SPA | HEADERS | REDIRECT)*

SPA          := '<spa' ATTR_LIST '/>'
HEADERS      := '<headers>' HEADER_TAG* '</headers>'
HEADER_TAG   := '<header' ATTR_LIST '/>'
REDIRECT     := '<redirect' ATTR_LIST '/>'

ALIAS        := '<alias' ATTR_LIST '/>'

PROP_LINE    := ATTR NEWLINE
ATTR_LIST    := (WS ATTR)*
ATTR         := KEY '=' VALUE

DOMAIN       := HOSTNAME | WILDCARD | '_default'
HOSTNAME     := [a-zA-Z0-9.-]+
WILDCARD     := '*.' HOSTNAME

KEY          := [a-zA-Z_][a-zA-Z0-9_-]*
VALUE        := STRING | ARRAY | BOOL | NUMBER
STRING       := '"' QUOTED '"'
QUOTED       := [^"]*
BOOL         := 'true' | 'false'
NUMBER       := [0-9]+
ARRAY        := '[' VALUE (',' VALUE)* ']'
```

---

## Supported Directives

### Global Defaults

| Directive | Type | Default | Description |
|-----------|------|---------|-------------|
| `ssl_min_version` | string | TLS1.2 | Minimum TLS version |
| `index` | array | [index.html] | Default index files |

### VHost Tag

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| `root` | path | auto | Document root directory |
| `cert` | path | auto | Certificate directory |
| `enabled` | bool | true | Enable/disable this vhost |

### SPA Tag (nested in vhost)

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| `enabled` | bool | false | Enable SPA mode |
| `routes` | array | [/] | Routes that trigger SPA fallback |
| `exclude` | array | [] | Paths that bypass SPA (e.g., /api) |

### Headers Tag (nested in vhost)

Custom HTTP response headers as `<header name="..." value="..."/>` children.

### Redirect Tag

```text
<redirect from="/old-path" to="/new-path" code="301" />
<redirect from="/blog/*" to="https://blog.example.com$1" code="302" />
```

---

## Implementation Plan

### Phase 1: Lexer

**File:** `include/neonsignal/vhscript/lexer.h++`

```cpp
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace neonsignal::vhscript {

enum class TokenType {
    // Symbols
    TagOpen,        // <
    TagClose,       // >
    Slash,          // /
    Equals,         // =
    BracketOpen,    // [
    BracketClose,   // ]
    Comma,          // ,

    // Values
    Identifier,     // tag names, attribute keys, domain names
    String,         // quoted string
    Number,         // integer

    // Special
    Comment,
    Eof,
    Error
};

struct Token {
    TokenType type;
    std::string_view text;
    std::size_t line;
    std::size_t column;
};

class Lexer {
public:
    explicit Lexer(std::string_view source);
    Token next();
    [[nodiscard]] bool has_errors() const;
    [[nodiscard]] std::vector<std::string> errors() const;

private:
    std::string_view source_;
    std::size_t pos_{0};
    std::size_t line_{1};
    std::size_t column_{1};
    std::vector<std::string> errors_;

    char peek() const;
    char peek_next() const;
    char advance();
    void skip_whitespace_and_comments();
    Token make_token(TokenType type, std::size_t start);
    Token scan_identifier_or_keyword();
    Token scan_string();
    Token scan_number();
};

} // namespace neonsignal::vhscript
```

### Phase 2: Parser

**File:** `include/neonsignal/vhscript/parser.h++`

```cpp
#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace neonsignal::vhscript {

using Value = std::variant<
    std::string,
    bool,
    int64_t,
    std::vector<std::string>
>;

struct RedirectRule {
    std::string from;
    std::string to;
    int code{301};
};

struct SpaConfig {
    bool enabled{false};
    std::vector<std::string> routes;
    std::vector<std::string> exclude;
};

struct VHostConfig {
    std::string domain;
    bool is_wildcard{false};
    bool is_default{false};
    bool enabled{true};

    std::optional<std::filesystem::path> root;
    std::optional<std::filesystem::path> cert;

    SpaConfig spa;
    std::vector<std::string> index_files;
    std::unordered_map<std::string, std::string> headers;
    std::vector<RedirectRule> redirects;
};

struct GlobalDefaults {
    std::string ssl_min_version{"TLS1.2"};
    std::vector<std::string> index_files{"index.html"};
};

struct VHScriptConfig {
    GlobalDefaults defaults;
    std::vector<VHostConfig> vhosts;
    std::unordered_map<std::string, std::string> aliases;
};

class Parser {
public:
    static std::optional<VHScriptConfig> parse_file(
        const std::filesystem::path& path);

    static std::optional<VHScriptConfig> parse_string(
        std::string_view source,
        const std::string& filename = "<string>");

    [[nodiscard]] std::vector<std::string> errors() const;

private:
    std::vector<std::string> errors_;

    bool parse_defaults(VHScriptConfig& config);
    bool parse_vhost(VHScriptConfig& config);
    bool parse_alias(VHScriptConfig& config);
    bool parse_spa(VHostConfig& vhost);
    bool parse_headers(VHostConfig& vhost);
    bool parse_redirect(VHostConfig& vhost);
    bool parse_header_tag(VHostConfig& vhost);
};

} // namespace neonsignal::vhscript
```

### Phase 3: VHost Manager

**File:** `include/neonsignal/vhost_manager.h++`

```cpp
#pragma once

#include "neonsignal/vhost.h++"
#include "neonsignal/vhscript/parser.h++"

#include <filesystem>
#include <optional>
#include <shared_mutex>

namespace neonsignal {

class VHostManager {
public:
    VHostManager(
        std::filesystem::path public_root,
        std::filesystem::path config_path = "config/vhosts.vhs");

    bool initialize();
    std::optional<vhscript::VHostConfig> resolve(std::string_view authority) const;
    bool reload();
    [[nodiscard]] std::vector<std::string> list_vhosts() const;

private:
    std::filesystem::path public_root_;
    std::filesystem::path config_path_;
    mutable std::shared_mutex mutex_;

    std::optional<vhscript::VHScriptConfig> config_;
    VHostResolver auto_resolver_;

    std::unordered_map<std::string, vhscript::VHostConfig> vhosts_;
    std::vector<vhscript::VHostConfig> wildcard_vhosts_;
    std::unordered_map<std::string, std::string> aliases_;

    void merge_auto_discovered();
    std::string normalize_domain(std::string_view domain) const;
};

} // namespace neonsignal
```

---

## File Structure

```
config/
└── vhosts.vhs                  # Main configuration file

include/neonsignal/
├── vhscript/
│   ├── lexer.h++               # Tokenizer
│   ├── parser.h++              # Parser + AST
│   └── types.h++               # Shared types
└── vhost_manager.h++           # Unified vhost resolution

src/neonsignal/
├── vhscript/
│   ├── lexer.c++
│   └── parser.c++
└── vhost_manager/
    ├── initialize.c++
    ├── resolve.c++
    └── reload.c++
```

---

## Configuration Loading Order

```
1. Check if config/vhosts.vhs exists
   ├── Yes: Parse VHScript
   │        ├── Valid: Use explicit vhost configurations
   │        └── Error: Log error, abort startup
   └── No: Skip to step 2

2. Auto-discover public/<domain>/ directories
   └── Add domains NOT already defined in VHScript

3. Merge configurations
   ├── VHScript vhosts take priority
   ├── Auto-discovered vhosts fill gaps
   └── _default is always last resort
```

---

## Startup Logging

```
neonsignal->VHScript: loaded config/vhosts.vhs
neonsignal->Virtual hosts:
  [config] simonedelpopolo.host {
      root: public/simonedelpopolo.host
      cert: certs/simonedelpopolo.host
      spa: enabled, routes: [/, /app, /dashboard]
      headers: 3
  }
  [config] neonsignal.nutsloop.host {
      root: public/neonsignal.nutsloop.host
      cert: certs/neonsignal.nutsloop.host
      spa: enabled, routes: [/, /auth, /admin]
  }
  [auto] _default {
      root: public/_default
      cert: certs/_default
  }
neonsignal->Aliases:
  www.simonedelpopolo.host -> simonedelpopolo.host
  simone.host -> simonedelpopolo.host
```

---

## Error Messages

```
config/vhosts.vhs:12:3: error: expected '>' after vhost binding
<simonedelpopolo.host=/home/core/code/neonsignal/public/simonedelpopolo.host
  ^
config/vhosts.vhs:18:1: error: unknown tag 'server'
<server domain="example.com">
 ^~~~~~
config/vhosts.vhs:24:1: error: mismatched closing tag (expected </simonedelpopolo.host>)
</simonedelpopolo.com>
^
```

---

## Future Enhancements

1. **Include directive** - Split config across files:
   ```text
   <include src="sites-enabled/*.vhs" />
   ```

2. **Environment variables** - Dynamic values:
   ```text
   <${PRIMARY_DOMAIN}=public/${PRIMARY_DOMAIN}>
     cert="certs/${PRIMARY_DOMAIN}"
   </${PRIMARY_DOMAIN}>
   ```

3. **Location blocks** - Path-specific rules:
   ```text
   <api.example.com=/var/www/api.example.com>
     <location path="/v1" proxy="http://localhost:3000" />
     <location path="/static" cache="7d" />
   </api.example.com>
   ```

4. **Rate limiting**:
   ```text
   <api.example.com=/var/www/api.example.com>
     <rate_limit requests="100" window="60s" burst="20" />
   </api.example.com>
   ```

5. **Access control**:
   ```text
   <admin.example.com=/var/www/admin.example.com>
     <access>
       <allow cidr="10.0.0.0/8" />
       <allow cidr="192.168.0.0/16" />
       <deny cidr="all" />
     </access>
   </admin.example.com>
   ```

---

## Implementation Order

1. **Lexer** - Tokenize VHScript syntax
2. **Parser** - Build AST from tokens
3. **VHostManager** - Merge VHScript + auto-discovery
4. **Integration** - Update Http2Listener and CertManager
5. **SIGHUP reload** - Runtime config updates
6. **Documentation** - User guide with examples

**Estimated effort:** 8-12 hours for core implementation
