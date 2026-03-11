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

## CI Gates
Triggers:
- Pull requests to `develop` and `master`
- Pushes to `develop` and `master`

Required checks (cloud runner):
- ESP-IDF build (`idf.py build`)
- Unit-test-app build for owned components
- Lint/format checks (when configured)

Required checks (self-hosted runner, planned HIL gate):
- Flash target board
- Run smoke/startup verification

Merge protection:
- Failing required checks block merge.

## Release and Tagging
- Release source branch: `master` only
- Tags are created only from commits already merged to `master`
- Tag format: `vX.Y.Z` (SemVer)
  - `MAJOR`: breaking changes
  - `MINOR`: backward-compatible features
  - `PATCH`: fixes/docs/tests
- Tag push (`v*`) triggers release workflow to:
  - Create GitHub Release
  - Attach firmware artifacts
  - Publish release notes/changelog summary

## Tracking and Definition of Done
Near-term milestones:
- Add `.github/workflows/ci.yml` and pass cloud checks on PRs.
- Document and provision self-hosted runner for HIL.
- Promote HIL job to required check for `develop` -> `master`.

Definition of done:
- CI workflow(s) committed and passing
- Branch protections enabled for `develop` and `master`
- README points to this workflow
- First release tag cut from `master` using this process
