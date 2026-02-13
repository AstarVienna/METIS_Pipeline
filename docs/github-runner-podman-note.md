#  GitHub Actions Self-Hosted Runner on Podman 

This document provides step-by-step instructions for deploying a self-hosted GitHub Actions runner inside a **Podman container** on a local Linux host named `zeus`.

---

##  Prerequisites

- Fedora Linux installed (version  38 recommended)
- Podman installed (`dnf install podman`)
- GitHub account with **admin access** to a repository (e.g., your fork of `AstarVienna/METIS_Pipeline`)
- Terminal access to the host (`Zeus`)
- Internet access from the host

---

##  1. Prepare the Workspace

```bash
mkdir -p ~/podman-gh-runner
cd ~/podman-gh-runner
```

---

##  2. Create `entrypoint.sh`

This script registers and starts the GitHub Actions runner inside the container:

```bash
#!/bin/bash
set -e

./config.sh \
  --url https://github.com/<your-username>/METIS_Pipeline \
  --token <your-registration-token> \
  --name podman-runner \
  --labels podman,linux \
  --unattended \
  --replace

exec ./run.sh
```

> Replace `<your-username>` and `<your-registration-token>` with your actual GitHub account and the one-time token from GitHub's runner setup page.  Please note, the token require admin previlage.


Make it executable:

```bash
chmod +x entrypoint.sh
```

---

##  3. Create the `Containerfile`

```Dockerfile
FROM fedora:latest

# Install required dependencies for .NET 6 runtime (used by GitHub Actions runner)
RUN dnf install -y \
    curl git tar jq procps \
    icu krb5-libs libunwind lttng-ust openssl-libs zlib libcurl \
    && dnf clean all

# Create non-root user (UID:GID = 1005:1005)
RUN groupadd -g 1005 runner && \
    useradd -m -u 1005 -g 1005 runner

# Prepare working directory
RUN mkdir -p /runner && chown -R runner:runner /runner
WORKDIR /runner

# Switch to non-root user
USER runner

# Download GitHub Actions runner
RUN curl -L -O https://github.com/actions/runner/releases/download/v2.314.1/actions-runner-linux-x64-2.314.1.tar.gz && \
    tar xzf actions-runner-linux-x64-2.314.1.tar.gz && \
    rm -f actions-runner-linux-x64-2.314.1.tar.gz

# Copy entrypoint script
COPY --chown=runner:runner entrypoint.sh /runner/entrypoint.sh
RUN chmod +x /runner/entrypoint.sh

ENTRYPOINT ["/runner/entrypoint.sh"]
```

---

##  4. Build the Container Image

```bash
podman build -t gh-runner-metis .
```

---

##  5. Run the Container

```bash
podman run -d --name metis-runner \
  --network host \
  gh-runner-metis
```

> `--network host` ensures the runner can connect to GitHub directly.

---

##  6. Verify Logs

```bash
podman logs -f metis-runner
```

Expected output:

```
 Connected to GitHub
 Runner successfully added
 Runner connection is good
 Listening for Jobs
```

---

##  7. Confirm in GitHub

Navigate to your repository:

> `https://github.com/<your-username>/METIS_Pipeline/settings/actions/runners`

You should see `podman-runner` listed as **Idle** and ready.

---

##  8. (Optional) Enable systemd Auto-Start

```bash
podman generate systemd --name metis-runner --files --restart-policy=always
sudo cp container-metis-runner.service /etc/systemd/system/
sudo systemctl enable --now container-metis-runner.service
```

---

##  Security Notes

- The runner executes workflows with the same privileges as the container user (UID 1005)
- Avoid embedding GitHub tokens directly in version-controlled files
- Use secrets, volumes, or environment variables for token injection in production environments

---

##  Cleanup

```bash
podman rm -f metis-runner
podman rmi gh-runner-metis
```

---

##  References

- [GitHub Actions Self-Hosted Runners](https://docs.github.com/en/actions/hosting-your-own-runners)
- [Fedora dependencies for .NET 6](https://learn.microsoft.com/en-us/dotnet/core/install/linux-fedora#dependencies)