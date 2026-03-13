#!/usr/bin/env bash
set -euo pipefail

GH_RUNNER_NAME="${GH_RUNNER_NAME:-$(hostname)-runner}"
GH_RUNNER_LABELS="${GH_RUNNER_LABELS:-self-hosted,linux,hil,esp32c6}"
GH_RUNNER_GROUP="${GH_RUNNER_GROUP:-Default}"
GH_RUNNER_WORKDIR="${GH_RUNNER_WORKDIR:-_work}"
GH_RUNNER_REPLACE="${GH_RUNNER_REPLACE:-true}"
RUNNER_HOME="/home/runner/actions-runner"
RUNNER_TEMPLATE="/opt/actions-runner-template"

replace_flag=""
if [[ "${GH_RUNNER_REPLACE}" == "true" ]]; then
  replace_flag="--replace"
fi

# Root phase: ensure writable runner home for mounted volumes, then drop privileges.
if [[ "${1:-}" != "--as-runner" ]]; then
  mkdir -p "${RUNNER_HOME}"
  chown -R runner:runner "${RUNNER_HOME}"
  exec sudo -Eu runner /entrypoint.sh --as-runner
fi

cd "${RUNNER_HOME}"

# First start with fresh volume: copy runner binaries/scripts into persistent runner home.
if [[ ! -x "${RUNNER_HOME}/config.sh" ]]; then
  cp -a "${RUNNER_TEMPLATE}/." "${RUNNER_HOME}/"
fi

# Ensure work/tool directories are present.
mkdir -p "${RUNNER_HOME}/${GH_RUNNER_WORKDIR}" "${RUNNER_HOME}/${GH_RUNNER_WORKDIR}/_tool"

# Load ESP-IDF environment so idf.py/tools are available to workflow steps.
if [[ -f /opt/esp/idf/export.sh ]]; then
  # shellcheck disable=SC1091
  source /opt/esp/idf/export.sh >/dev/null
else
  echo "ERROR: /opt/esp/idf/export.sh not found in runner image"
  exit 1
fi

# Configure only once; subsequent container starts should reuse existing config.
if [[ ! -f .runner ]]; then
  : "${GH_REPO_URL:?GH_REPO_URL is required (example: https://github.com/JimmieKraatz/blinky_c6)}"
  : "${GH_RUNNER_TOKEN:?GH_RUNNER_TOKEN is required (use New self-hosted runner token from GitHub settings)}"
  echo "Configuring runner: ${GH_RUNNER_NAME}"
  ./config.sh \
    --unattended \
    --url "${GH_REPO_URL}" \
    --token "${GH_RUNNER_TOKEN}" \
    --name "${GH_RUNNER_NAME}" \
    --labels "${GH_RUNNER_LABELS}" \
    --runnergroup "${GH_RUNNER_GROUP}" \
    --work "${GH_RUNNER_WORKDIR}" \
    ${replace_flag}
else
  echo "Runner already configured; skipping config.sh"
fi

echo "Starting runner"
exec ./run.sh
