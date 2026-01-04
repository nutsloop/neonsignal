# Virtual Hosting

Neonsignal supports name-based virtual hosting, allowing a single server instance to serve different content and TLS certificates based on the requested hostname.

- **Content:** The server uses the HTTP/2 `:authority` pseudo-header (equivalent to the HTTP/1.1 `Host` header) to serve content from different document roots.
- **Certificates:** The server uses the SNI (Server Name Indication) extension during the TLS handshake to provide per-domain SSL/TLS certificates.

## Content Hosting

The server can automatically discover virtual hosts by scanning the `public/` directory for subdirectories that are named like domains.

### Directory Structure

A flat directory structure is used, where each subdirectory in `public/` corresponds to a domain:

```
public/
├── simonedelpopolo.host/
│   ├── index.html
│   └── app.js
├── api.simonedelpopolo.host/
│   └── index.html
└── _default/                    # Fallback for unknown domains
    └── index.html
```

### Domain Matching Strategy

The server resolves domains with the following priority:

1.  **Exact Match:** `api.simonedelpopolo.host` maps to `public/api.simonedelpopolo.host/`.
2.  **Wildcard Match:** A request for `app.simonedelpopolo.host` would map to a `public/*.simonedelpopolo.host/` directory if it existed.
3.  **Default:** Any unknown domain maps to `public/_default/`.
4.  **Legacy Fallback:** If no virtual host directories are found, the server falls back to serving content from the `public/` directory itself.

## Per-Domain TLS Certificates (SNI)

Neonsignal extends the virtual hosting concept to TLS certificates, allowing each domain to have its own SSL/TLS certificate. This is accomplished using SNI.

### Certificate Directory Structure

The certificate directory structure mirrors the content directory structure:

```
certs/
├── simonedelpopolo.host/
│   ├── fullchain.pem      # Certificate chain
│   └── privkey.pem        # Private key
├── neonsignal.nutsloop.host/
│   ├── fullchain.pem
│   └── privkey.pem
└── _default/              # Fallback certificate
    ├── fullchain.pem
    └── privkey.pem
```

At startup, the server's `CertManager` scans the `certs/` directory, pre-loading an `SSL_CTX` for each domain found. During the TLS handshake, an SNI callback selects the appropriate context based on the hostname provided by the client.

## VHScript: Advanced Configuration

```{admonition} Planned Feature
:class: warning

VHScript is currently in the design phase and **not yet implemented**. The syntax and features described below represent the planned specification. Contributions welcome!
```

For more explicit control over virtual host behavior, Neonsignal provides a lightweight configuration language called VHScript. This allows for fine-tuning, such as defining SPAs, setting custom headers, and creating redirects.

### Configuration Priority

1.  **VHScript declarations (`config/vhosts.vhs`)** - Highest priority
2.  **Auto-discovered directories (`public/<domain>`)**
3.  **`_default` fallback** - Lowest priority

### Syntax Example

VHScript uses a simple, XML-inspired syntax.

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
    <header name="Strict-Transport-Security"
            value="max-age=31536000; includeSubDomains" />
  </headers>

  <redirect from="/old-path" to="/new-path" code="301" />
</simonedelpopolo.host>

<_default=/home/core/code/neonsignal/public/_default>
  cert="/home/-core/code/neonsignal/certs/_default"
</_default>
```

### Supported Directives

-   **`<defaults>`:** Sets global defaults for all virtual hosts.
-   **`<domain=/path/to/root>`:** Defines a virtual host, binding a domain to a document root. The `cert` attribute specifies the path to the certificate directory.
-   **`<spa>`:** Configures a host to act as a Single Page Application, defining which routes should fall back to the index page.
-   **`<headers>`:** Allows setting custom HTTP response headers.
-   **`<redirect>`:** Defines HTTP redirects.
-   **`<alias>`:** Allows multiple domains to point to the same configuration.
