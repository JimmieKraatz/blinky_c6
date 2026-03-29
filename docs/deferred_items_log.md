# Deferred Items Log

Use this file only for deferred items that cannot yet be resolved in the current slice and do not yet have a clear owning future slice.

This is a tracking log, not a substitute for recording the deferral decision in `DEVLOG.md`. The development journal should record the action of deferring; this log tracks the deferred item until it gains a clear future owner or is resolved.

## Purpose

- Track unresolved items that cannot yet be resolved or cleanly delegated to an open slice.
- Keep deferred work visible without overloading `DEVLOG.md`.
- Preserve both open and closed deferred items as a time-series record.

## Usage Rules

- Prefer assigning unresolved work to an open slice or sub-slice when it has a clear home.
- Create a deferred item only when the work cannot yet be resolved or cleanly delegated.
- Logging a deferred item is itself a real event and may be noted in `DEVLOG.md`.
- Do not delete resolved items; move them to the closed table and record how they were disposed.
- Keep `Open Items` ordered oldest first.
- Keep `Closed Items` ordered newest first.

## Open Items

| Id | Opened | Age Days | Summary | Source | Planned Disposition | Status |
| --- | --- | ---: | --- | --- | --- | --- |
| D-2026-03-27-02 | 2026-03-13 | 14 | Development-environment decoupling so native ESP-IDF usage stays first-class and the dev container becomes optional | Narrowed successor to `D-2026-03-13-02` after deferred-log review | Future environment decoupling slice | Open |
| D-2026-03-13-03 | 2026-03-13 | 14 | Bootstrap layering split between environment/bootstrap and runtime orchestration | Historical deferred items migrated from `DEVLOG.md` | Future architecture cleanup slice | Open |
| D-2026-03-27-03 | 2026-03-13 | 14 | Assert-contract and async/backpressure test-hardening follow-up after initial queue/runtime coverage landed | Narrowed successor to `D-2026-03-13-04` after deferred-log review | Future test-hardening branch | Open |
| D-2026-03-13-05 | 2026-03-13 | 14 | Runtime context ownership follow-up for `app_main` static context workaround | Historical deferred items migrated from `DEVLOG.md` | Future lifecycle ownership slice | Open |
| D-2026-03-13-06 | 2026-03-13 | 14 | Fault and shutdown semantics contract before adding new platform producers | Historical deferred items migrated from `DEVLOG.md` | Future fault-handling design slice | Open |
| D-2026-03-27-01 | 2026-03-27 | 0 | HIL gate maturation and enforcement follow-through after initial CI/CD rollout | Deferred-log review after Slice 5 docs/release closeout | Future CI/CD/HIL hardening slice | Open |
| D-2026-03-25-01 | 2026-03-25 | 2 | CLI presentation mode and response-formatting follow-up after config-command plumbing stabilizes | Slice 4d CLI config-command discussion | Future CLI presentation slice after plumbing is stable | Open |
| D-2026-03-25-02 | 2026-03-25 | 2 | Review whether per-setting config read commands should be added alongside grouped `config show` views | Slice 4d read-command validation and CLI discussion | Future CLI UX follow-up after current Slice 4d scope closes | Open |
| D-2026-03-29-01 | 2026-03-29 | 0 | Clarify in workflow/docs that slice numbering is branch-local and new branches restart at `Slice 1` | Hotfix branch workflow practice during RC release-version fix | Future workflow/docs clarification slice spanning development guide and agent guidance | Open |

## Closed Items

| Id | Opened | Closed | Age Days | Summary | Source | Disposition | Status |
| --- | --- | --- | ---: | --- | --- | --- | --- |
| D-2026-03-13-04 | 2026-03-13 | 2026-03-27 | 14 | Dedicated test-hardening branch for async timing, overflow, and assert-contract coverage | Historical deferred items migrated from `DEVLOG.md` | Partially satisfied by later queue/runtime coverage and overflow instrumentation; remaining scope narrowed into `D-2026-03-27-03` for assert-contract and async/backpressure hardening follow-through. | Closed |
| D-2026-03-13-02 | 2026-03-13 | 2026-03-27 | 14 | Dev-ability and reproducibility hardening for in-repo onboarding/build setup | Historical deferred items migrated from `DEVLOG.md` | Partially satisfied by later README, workflow, runner, and devcontainer-related improvements; remaining scope narrowed into `D-2026-03-27-02` for development-environment decoupling so native ESP-IDF usage stays first-class and the dev container becomes optional. | Closed |
| D-2026-03-13-01 | 2026-03-13 | 2026-03-27 | 14 | CI/CD hardware-constrained rollout and HIL gate follow-through | Historical deferred items migrated from `DEVLOG.md` | Closed after CI app build, unit-test-app build, release automation, and manual HIL smoke rollout landed; remaining work narrowed into `D-2026-03-27-01` for HIL gate maturation and enforcement follow-through. | Closed |
| D-2026-03-13-08 | 2026-03-13 | 2026-03-14 | 1 | Pause output behavior implementation once pause policy is finalized | Historical deferred items migrated from `DEVLOG.md` | Resolved by landed LED-off-on-pause behavior in `d8c1ea9`; regression-hardened in `60ba9e9` with same-step pause output suppression coverage. | Closed |
| D-2026-03-13-07 | 2026-03-13 | 2026-03-14 | 1 | Pause behavior policy decision for freeze-brightness vs LED-off semantics | Historical deferred items migrated from `DEVLOG.md` | Resolved in practice by `d8c1ea9`, which established paused-state LED-off semantics; later documented and regression-hardened in `60ba9e9`. | Closed |

## Notes

- Use stable IDs so journal references remain easy to follow.
- If a deferred item later gains a clear owner, update the disposition and move it to `Closed Items` once the handoff is complete.
- Prefer specific summaries over vague labels like "cleanup later" or "maybe rename."
