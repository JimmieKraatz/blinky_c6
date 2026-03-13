#!/usr/bin/env bash
set -euo pipefail

cd /home/runner/actions-runner

: "${GH_REPO_URL:?GH_REPO_URL is required (example: https://github.com/JimmieKraatz/blinky_c6)}"
: "${GH_RUNNER_TOKEN:?GH_RUNNER_TOKEN is required (use New self-hosted runner token from GitHub settings)}"

GH_RUNNER_NAME="${GH_RUNNER_NAME:-$(hostname)-runner}"
GH_RUNNER_LABELS="${GH_RUNNER_LABELS:-self-hosted,linux,hil,esp32c6}"
GH_RUNNER_GROUP="${GH_RUNNER_GROUP:-Default}"
GH_RUNNER_WORKDIR="${GH_RUNNER_WORKDIR:-_work}"
GH_RUNNER_REPLACE="${GH_RUNNER_REPLACE:-true}"

replace_flag=""
if [[ "${GH_RUNNER_REPLACE}" == "true" ]]; then
  replace_flag="--replace"
fi

# Ensure work/tool directories are present and writable for the runner user.
mkdir -p "${GH_RUNNER_WORKDIR}" "${GH_RUNNER_WORKDIR}/_tool"

# Configure only once; subsequent container starts should reuse existing config.
if [[ ! -f .runner ]]; then
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
