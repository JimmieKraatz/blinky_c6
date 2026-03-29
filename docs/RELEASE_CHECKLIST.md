# Release Checklist

Use this checklist for every release candidate and final release.

## Scope
- Release source branch: `master`
- Versioning: SemVer (`vMAJOR.MINOR.PATCH`)
- RC format: `vMAJOR.MINOR.PATCH-rc.N`

## 1) Pre-Release Readiness
- [ ] `develop` is merged into `master` through PR with required cloud checks green.
- [ ] `master` back-merged into `develop` if a unique merge commit was created.
- [ ] Working tree is clean locally.

## 2) RC Cut (`vX.Y.Z-rc.N`)
- [ ] Create annotated RC tag on `master` commit:
  - `git tag -a vX.Y.Z-rc.N -m "Release candidate vX.Y.Z-rc.N"`
- [ ] Push tag:
  - `git push origin vX.Y.Z-rc.N`
- [ ] Confirm `Release` workflow completes.
- [ ] Confirm pre-release is created with expected artifacts:
  - `blinky_c6.bin`
  - `blinky_c6.elf`
  - `bootloader.bin`
  - `partition-table.bin`
  - `flasher_args.json`
  - optional map file
  - `sha256sums.txt`

## 3) HIL Validation (Required Before Final Tag)
- [ ] Run `HIL Smoke` manually on the exact RC commit.
- [ ] Confirm flash + monitor startup validation pass.
- [ ] Confirm run logs/artifacts are retained for traceability.

## 4) Final Release (`vX.Y.Z`)
- [ ] If RC commit is accepted unchanged, tag the same commit:
  - `git tag -a vX.Y.Z -m "Release vX.Y.Z"`
- [ ] Push final tag:
  - `git push origin vX.Y.Z`
- [ ] Confirm final (non-pre-release) GitHub Release is created.
- [ ] Verify release notes and assets/checksums.

## 5) Post-Release Hygiene
- [ ] Ensure `develop` and `master` are synchronized (PR `master -> develop` if needed).
- [ ] Update `docs/DEVLOG.md` with release summary and validation notes.
- [ ] Delete temporary validation tags/releases if any were used.

