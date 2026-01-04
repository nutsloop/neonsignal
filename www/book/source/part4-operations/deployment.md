# Deployment

This guide covers the process of deploying the Neonsignal server to a production environment.

## Deployment Workflow

The recommended deployment process involves:

1.  **Building:** Compiling the frontend and backend in release mode.
2.  **Syncing:** Transferring the built project to the production server.
3.  **Installing:** Placing the binaries in a system directory and setting up the `systemd` services.

### Building for Production

The `scripts/re-neonsignal-all.sh` script automates the entire build and local installation process. It performs the following steps:

1.  Builds the frontend assets.
2.  Configures the Meson build system for a release build (`-Dbuildtype=release`).
3.  Compiles the C++ backend with optimizations.
4.  Strips debug symbols from the binaries to reduce their size.
5.  Calls the `install.sh` script to deploy the binaries and services locally.

**Usage:**

```bash
./scripts/re-neonsignal-all.sh
```

### Syncing Files to the Server

The `scripts/rsync_deploy.sh` script is provided to efficiently sync the project files to a remote server. It uses `rsync` to transfer only the changed files, and it respects the project's `.gitignore` file to avoid syncing unnecessary files.

**Usage:**

Before running, you may need to configure the `REMOTE` and `SSH_KEY` variables in the script.

```bash
# Perform a dry run to see what will be synced
DRY_RUN=1 ./scripts/rsync_deploy.sh

# Perform the actual sync
./scripts/rsync_deploy.sh
```

### Installing the Service

The `scripts/install.sh` script handles the installation of the server binaries and `systemd` unit files. It is typically called by the build script, but can also be run manually on the production server.

The script performs these actions:

1.  Stops the `neonsignal` and `neonsignal-redirect` services.
2.  Copies the compiled binaries to `/usr/local/bin/`.
3.  Sets the correct ownership and permissions.
4.  **Restores SELinux contexts**, which is crucial for security on systems like Fedora or CentOS.
5.  Updates the `ExecStart` path in the `systemd` service files to point to the new binary location.
6.  Reloads the `systemd` daemon and starts the services.

### Managing the Services

Once installed, the services can be managed with standard `systemctl` commands.

**Install and Enable Services:**

```bash
# Copy the unit files
sudo install -m 0644 systemd/neonsignal.service /etc/systemd/system/
sudo install -m 0644 systemd/neonsignal-redirect.service /etc/systemd/system/
sudo systemctl daemon-reload

# Enable the services to start on boot
sudo systemctl enable --now neonsignal.service neonsignal-redirect.service
```

**Check Service Status:**

```bash
sudo systemctl status neonsignal.service neonsignal-redirect.service
```

**Watch Logs:**

```bash
sudo journalctl -u neonsignal.service -u neonsignal-redirect.service -f
```
