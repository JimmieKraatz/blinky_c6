FROM espressif/idf:release-v5.5

ARG RUNNER_VERSION=2.329.0

USER root

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        git \
        jq \
        libicu-dev \
        sudo \
        tini \
        unzip \
        xz-utils \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -d /home/runner -s /bin/bash runner \
    && echo "runner ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/runner \
    && chmod 0440 /etc/sudoers.d/runner

WORKDIR /home/runner/actions-runner

RUN set -eux; \
    arch="$(uname -m)"; \
    case "$arch" in \
      x86_64) runner_arch="x64" ;; \
      aarch64) runner_arch="arm64" ;; \
      armv7l) runner_arch="arm" ;; \
      *) echo "Unsupported architecture: $arch"; exit 1 ;; \
    esac; \
    curl -fsSL -o actions-runner.tar.gz \
      "https://github.com/actions/runner/releases/download/v${RUNNER_VERSION}/actions-runner-linux-${runner_arch}-${RUNNER_VERSION}.tar.gz"; \
    tar xzf actions-runner.tar.gz; \
    rm -f actions-runner.tar.gz; \
    ./bin/installdependencies.sh

COPY infra/runner/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh \
    && chown -R runner:runner /home/runner/actions-runner

USER runner

ENTRYPOINT ["/usr/bin/tini", "--", "/entrypoint.sh"]
