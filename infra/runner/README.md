# Self-Hosted GitHub Runner (Containerized)

This folder provisions a dedicated GitHub Actions self-hosted runner container for HIL workflows.

## Why this exists
- Isolates runner concerns from the daily dev container
- Keeps HIL runner setup reproducible in-repo
- Supports the `HIL Smoke` manual workflow in `.github/workflows/hil-smoke.yml`

## Prerequisites
- Docker + Docker Compose on runner host
- USB serial device accessible on host (default `/dev/ttyACM0`)
- GitHub repo admin access to generate runner registration token

## Setup
1. Copy env template:
   ```bash
   cp infra/runner/.env.example infra/runner/.env
   ```
2. In GitHub repo:
   - `Settings -> Actions -> Runners -> New self-hosted runner`
   - copy a fresh registration token
3. Paste token into `infra/runner/.env` as `GH_RUNNER_TOKEN`
4. Start runner:
   ```bash
   docker compose --env-file infra/runner/.env -f infra/runner/docker-compose.yml up -d --build
   ```
5. Verify runner is `Online` in GitHub UI.

## Validate with HIL workflow
Run `HIL Smoke` manually from Actions using branch `develop` and default inputs.

## Notes
- Registration tokens are short-lived. If container restart fails registration, generate a new token.
- Keep this runner dedicated to this repo/workload.
- Avoid mounting Docker socket unless absolutely required.
- Container restarts do not re-register the runner; existing `.runner` config is reused automatically.
- Runner startup sources `/opt/esp/idf/export.sh`, so `idf.py` is available to workflow jobs by default.
