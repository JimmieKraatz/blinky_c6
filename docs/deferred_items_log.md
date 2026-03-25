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
| D-2026-03-13-01 | 2026-03-13 | 12 | CI/CD hardware-constrained rollout and HIL gate follow-through | Historical deferred items migrated from `DEVLOG.md` | Future CI/CD follow-up slice | Open |
| D-2026-03-13-02 | 2026-03-13 | 12 | Dev-ability and reproducibility hardening for in-repo onboarding/build setup | Historical deferred items migrated from `DEVLOG.md` | Future environment hardening slice | Open |
| D-2026-03-13-03 | 2026-03-13 | 12 | Bootstrap layering split between environment/bootstrap and runtime orchestration | Historical deferred items migrated from `DEVLOG.md` | Future architecture cleanup slice | Open |
| D-2026-03-13-04 | 2026-03-13 | 12 | Dedicated test-hardening branch for async timing, overflow, and assert-contract coverage | Historical deferred items migrated from `DEVLOG.md` | Future test-hardening branch | Open |
| D-2026-03-13-05 | 2026-03-13 | 12 | Runtime context ownership follow-up for `app_main` static context workaround | Historical deferred items migrated from `DEVLOG.md` | Future lifecycle ownership slice | Open |
| D-2026-03-13-06 | 2026-03-13 | 12 | Fault and shutdown semantics contract before adding new platform producers | Historical deferred items migrated from `DEVLOG.md` | Future fault-handling design slice | Open |
| D-2026-03-13-07 | 2026-03-13 | 12 | Pause behavior policy decision for freeze-brightness vs LED-off semantics | Historical deferred items migrated from `DEVLOG.md` | Future pause-policy decision slice | Open |
| D-2026-03-13-08 | 2026-03-13 | 12 | Pause output behavior implementation once pause policy is finalized | Historical deferred items migrated from `DEVLOG.md` | Future LED-domain behavior slice after policy decision | Open |
| D-2026-03-25-01 | 2026-03-25 | 0 | CLI presentation mode and response-formatting follow-up after config-command plumbing stabilizes | Slice 4d CLI config-command discussion | Future CLI presentation slice after plumbing is stable | Open |
| D-2026-03-25-02 | 2026-03-25 | 0 | Review whether per-setting config read commands should be added alongside grouped `config show` views | Slice 4d read-command validation and CLI discussion | Future CLI UX follow-up after current Slice 4d scope closes | Open |

## Closed Items

| Id | Opened | Closed | Age Days | Summary | Source | Disposition | Status |
| --- | --- | --- | ---: | --- | --- | --- | --- |

## Notes

- Use stable IDs so journal references remain easy to follow.
- If a deferred item later gains a clear owner, update the disposition and move it to `Closed Items` once the handoff is complete.
- Prefer specific summaries over vague labels like "cleanup later" or "maybe rename."
