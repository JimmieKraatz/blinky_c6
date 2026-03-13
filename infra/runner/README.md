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
4. Set serial device group id in `infra/runner/.env`:
   ```bash
   stat -c '%g' /dev/ttyACM0
   ```
   Put that value into `HIL_SERIAL_GID`.
5. Start runner:
   ```bash
   docker compose --env-file infra/runner/.env -f infra/runner/docker-compose.yml up -d --build
   ```
6. Verify runner is `Online` in GitHub UI.

## One-time migration note
If you used an older runner compose that mounted only `_work`, do a reset once:
```bash
docker compose --env-file infra/runner/.env -f infra/runner/docker-compose.yml down
docker volume rm blinky_c6_runner-data || true
docker volume rm blinky_c6_runner-home || true
docker compose --env-file infra/runner/.env -f infra/runner/docker-compose.yml up -d --build
```

## Validate with HIL workflow
Run `HIL Smoke` manually from Actions using branch `develop` and default inputs.

## Notes
- Registration tokens are short-lived. If container restart fails registration, generate a new token.
- `GH_RUNNER_TOKEN` is only required when runner registration is first created (or when `.runner` state is reset).
- Keep this runner dedicated to this repo/workload.
- Avoid mounting Docker socket unless absolutely required.
- Container restarts do not re-register the runner; existing `.runner` config is reused automatically.
- Runner startup sources `/opt/esp/idf/export.sh`, so `idf.py` is available to workflow jobs by default.
- If flash fails with `Path '/dev/ttyACM0' is not readable`, verify `HIL_SERIAL_GID` matches host device group id and recreate container.
- Runner status in GitHub can remain `Online/Idle` briefly after container stop. Restarting before it flips `Offline` may produce temporary `A session for this runner already exists` messages; this usually self-resolves once prior session timeout completes.
