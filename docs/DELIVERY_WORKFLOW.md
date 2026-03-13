# Delivery Workflow

## Purpose
Define branch flow, CI/CD gates, and release/tag rules for this repository.

## Constraint and Mitigation (Current)
This project targets ESP32-C6 hardware and is developed on a Linux non-x86 devkit environment. Hardware-dependent validation (flash/run on attached board) is not fully portable to standard cloud runners.

Mitigation plan:
- Run hardware-independent checks in cloud GitHub Actions.
- Add a self-hosted GitHub Actions runner on the devkit host for hardware-in-the-loop (HIL) jobs.
- Treat this as an infrastructure milestone, not a quality exemption.

## Branching Model
- Long-lived branches: `develop`, `master`
- Working branches: `feature/<name>`, `fix/<name>`, `hotfix/<name>`
- Merge flow:
  - `feature/*` -> `develop`
  - `develop` -> `master`
  - `hotfix/*` -> `master`, then back-merge into `develop`
- Policy: no direct pushes to `master`

## Versioning and Release Cadence
- Version format: `vMAJOR.MINOR.PATCH` (SemVer)
  - `MAJOR`: breaking changes
  - `MINOR`: backward-compatible features
  - `PATCH`: fixes/docs/tests
- Pre-release candidate format (optional): `vMAJOR.MINOR.PATCH-rc.N`
- Release source branch: `master` only
- Tags are created only from commits already merged to `master`

## Workflow Stubs (What We Can Add Now)
Add these workflows even before full HIL automation:
- `.github/workflows/ci.yml`
  - Cloud checks for compile/integration confidence
- `.github/workflows/release.yml`
  - Release creation and artifact attachment on tag
- `.github/workflows/hil-smoke.yml`
  - Manual/dispatch-only HIL job skeleton on `self-hosted` runner label
  - Starts non-required until runner infrastructure is stable

## Triggers and Feedback
Recommended triggers:
- `pull_request` on `develop`, `master`
- `push` on `develop`, `master`
- `push` tags `v*`
- `workflow_dispatch` for manual HIL runs

Feedback surfaces:
- PR check status (required/optional checks)
- GitHub Checks annotations (build errors shown inline in PR)
- Action logs for detailed failure context
- Artifacts (firmware binaries, optional build logs/maps)
- GitHub Release page for tagged builds

## CI Gates
Required checks (cloud runner):
- ESP-IDF app build (`idf.py build`)
- Unit-test-app build for owned components:
  - `core_sm`
  - `core_blinky`
  - `blinky_idf`
  - `blinky_interfaces`
- Lint/format checks (when configured)

Planned required checks (self-hosted HIL gate):
- Flash target board
- Run startup/smoke verification over serial
- Optional scripted menu/button behavior smoke checks

Merge protection:
- Failing required checks block merge.
- `master` requires clean checks from `develop` promotion PR.
- Required check names:
  - `App Build (ESP-IDF)`
  - `Unit Test App Build (ESP-IDF)`

## Release and Tagging
- Tag push (`v*`) triggers release workflow to:
  - Create GitHub Release
  - Attach firmware artifacts
  - Publish release notes/changelog summary

Binary publishing guidance:
- Source remains the primary delivery artifact.
- Stable tags (`vX.Y.Z`) should publish flashable binaries as release assets.
- Recommended assets for stable tags:
  - firmware binary/binaries (`.bin`)
  - optional ELF/map files for debugging
  - checksum file (for example `sha256sums.txt`)
  - short flash instructions in release notes
- RC tags (`vX.Y.Z-rc.N`) may publish binaries marked as pre-release for validation/testing only.

Release-time quality gate (current policy):
- Before creating any release tag (`v*`), run `HIL Smoke` manually on the same commit and require success.
- Until HIL is promoted to required automated gate, this is a mandatory operator checklist item.

## RC Mechanics (When Needed)
- RC tags may be cut as `vX.Y.Z-rc.N` for release validation.
- RC validation runs the same cloud checks as release tags.
- Final `vX.Y.Z` tag is created from `master` after RC acceptance.
- No code changes between accepted RC and final tag unless explicitly documented.

## HIL Automation Plan and Difficulty
HIL automation is feasible; complexity is moderate and mostly operational.

Phase 1 (Low effort):
- Stand up self-hosted runner on devkit host
- Validate runner can execute `idf.py`, flash port access, and serial read
- Run via `workflow_dispatch` only

Phase 2 (Medium effort):
- Add deterministic smoke script:
  - flash firmware
  - wait for boot banner/log signature
  - pass/fail with timeout
- Upload serial log artifact on failure

Phase 3 (Medium/High effort):
- Add hardware reset control and flaky-run mitigation
- Promote HIL check to required for `develop` -> `master`
- Optionally add scheduled nightly HIL run

## Tracking and Definition of Done
Near-term milestones:
- Add `.github/workflows/ci.yml` and pass cloud checks on PRs.
- Add `.github/workflows/release.yml` for tag-driven releases.
- Add `.github/workflows/hil-smoke.yml` as manual self-hosted stub.
- Document and provision self-hosted runner for HIL.
- Enforce branch protections:
  - `develop`: require cloud checks
  - `master`: require cloud checks and PR-based merges only
- Promote HIL job to required check for `develop` -> `master` after stability period.

Definition of done:
- CI workflow(s) committed and passing
- Branch protections enabled for `develop` and `master`
- README points to this workflow
- First release tag cut from `master` using this process
