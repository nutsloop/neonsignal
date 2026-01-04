# Part 4: Operations & Deployment

This section provides practical guides for setting up, deploying, and maintaining a Neonsignal server.

## Certificate Management

Neonsignal requires TLS certificates to operate. This guide covers setting up certificates for both local development and production environments. The server is designed to work with per-domain certificates for its virtual hosting feature.

### Certificate Directory Structure

All certificates should be placed in the `certs/` directory, organized by domain name:

```
certs/
├── simonedelpopolo.host/
│   ├── fullchain.pem
│   └── privkey.pem
└── _default/
    ├── fullchain.pem
    └── privkey.pem
```

### Local Development Certificates

For local development, you can generate self-signed certificates.

#### Option 1: Basic Self-Signed Certificate (for a single host)

This one-liner generates a self-signed certificate valid for `localhost`.

```bash
mkdir -p certs && openssl req -x509 -newkey rsa:2048 -nodes \
  -keyout certs/server.key -out certs/server.crt -days 365 \
  -subj "/CN=localhost" && chmod 600 certs/server.key
```

#### Option 2: Using `mkcert_local.sh` for Multiple Virtual Hosts

The `scripts/mkcert_local.sh` script is the recommended way to generate certificates for multiple local virtual hosts. It creates a local Certificate Authority (CA) and then issues trusted certificates for each host defined in the script.

**Usage:**

1.  **Customize Hosts:** Edit `scripts/mkcert_local.sh` to define the virtual hosts you need.
2.  **Run the Script:**
    ```bash
    ./scripts/mkcert_local.sh
    ```
3.  **Trust the CA:** Import the generated CA certificate at `certs/ca/root.crt` into your operating system and/or browser and trust it. This will make all certificates issued by it trusted for local development.

### Production Certificates with Let's Encrypt

For production, it is highly recommended to use certificates from a trusted authority like Let's Encrypt. The `scripts/letsencrypt.sh` script is designed to automate this process using `certbot`.

#### Prerequisites

-   `certbot` must be installed (`sudo dnf install certbot`).
-   Your server must be accessible from the public internet on port 80 for the HTTP-01 challenge.
-   The `neonsignal_redirect` service must be running to handle the challenge requests.

#### Usage

The script is designed to manage certificates for multiple domains.

1.  **Configure Domains:** Edit `scripts/letsencrypt.sh` to define the domains and certificate names.
2.  **Request Certificates (Dry Run):**
    ```bash
    sudo ./scripts/letsencrypt.sh --dry-run request-all
    ```
    This will simulate the process without issuing real certificates.
3.  **Request Real Certificates:**
    ```bash
    sudo ./scripts/letsencrypt.sh request-all
    ```
    This will contact Let's Encrypt, perform the validation, and place the certificates in the correct directories within `certs/`.
4.  **Automated Renewal:** The script can also install a post-renewal hook for `certbot`. This hook will automatically restart the `neonsignal` service after a certificate is renewed, ensuring the new certificate is loaded.
    ```bash
    sudo ./scripts/letsencrypt.sh install-hook
    ```

After running the script, the `certs/` directory will contain subdirectories for each of your production domains, with the appropriate `fullchain.pem` and `privkey.pem` files.
