# Critical Review - 2026-03-11

Purpose: track high-criticality findings, remediation slices, and closure evidence.

## Scope Reviewed
- `components/core_blinky/*`
- `components/blinky_interfaces/*`
- `components/blinky_idf/*`
- `main/main.c`
- `docs/*`, `README.md`

## Findings Register

| ID | Severity | Area | Finding | Status |
|---|---|---|---|---|
| CR-001 | High | consumer lifecycle | `led_sm_consumer_*` task ownership is singleton-style behind context-shaped API; `ctx` ignored in notify/stop path | Done |
| CR-002 | High | startup ordering | Startup sequence has potential race between boot event consumer path and boot-pattern LED writes | Done |
| CR-003 | High | lifecycle semantics | `stop/start` semantics can preserve stale queue/runtime state; restart behavior not explicitly defined or tested | Done |
| CR-004 | Medium | config normalization | `_idf` timing values (`producer_poll_ms`, `boot_pattern_ms`) are not normalized at mapper boundary | Done |
| CR-005 | Medium | contract drift | `led_sm_step` header comment no longer matches implementation after async split | Done |
| CR-006 | Medium | API defensiveness | Null-safety expectations are inconsistent across core runtime/model APIs | Open |
| CR-007 | Medium | test depth | Missing targeted assertions for async lifecycle edge cases and log adapter behavior | Open |

### Resolution References (Done Findings: Git Commit Hashes)
Values below are abbreviated Git commit hashes.
- `CR-001`: `77339b5`
- `CR-002`: `ad112ac`
- `CR-003`: `77339b5`, `4cf6574`
- `CR-004`: `d78aecb`
- `CR-005`: `d35edf2`

## Planned Remediation Slices

1. Consumer Ownership and Lifecycle Contract
- Address: CR-001, CR-003
- Goal:
  - make ownership explicit (true singleton API, or context-owned task handle)
  - define and enforce restart semantics (`fresh` vs `resume`)
  - add deterministic tests for stop/start behavior
- Status:
  - completed in this branch:
    - task ownership moved to `sm_led_ctx_t` (non-singleton)
    - lifecycle API now supports `led_sm_start(..., LED_SM_START_FRESH|LED_SM_START_RESUME)`
    - targeted tests added for fresh vs resume and start idempotence
  - commits: `77339b5`, `4cf6574`

2. Startup Ordering and Output Serialization
- Address: CR-002
- Goal:
  - remove boot-pattern/consumer output race
  - preserve expected boot indication behavior
  - add one integration-level ordering test
- Status:
  - completed in this branch:
    - fresh-start ordering changed to run boot pattern before consumer task start
    - boot event is enqueued after task creation, removing startup pattern/output interleaving window
  - note:
    - on-target run is still recommended to validate visual/log ordering behavior
  - commits: `ad112ac`

3. Config Boundary Hardening
- Address: CR-004, CR-005
- Goal:
  - normalize/clamp `_idf` timing values at config mapper boundary
  - align API comments/docs with actual async behavior
- Status:
  - completed in this branch:
    - mapper normalization/clamp added for `producer_poll_ms` and `boot_pattern_ms` (min/max)
    - Kconfig ranges added as UI guardrails for both symbols
    - targeted mapper clamp tests added
  - commits: `d78aecb`

4. Core API Consistency and Test Hardening
- Address: CR-006, CR-007
- Goal:
  - decide and document null-contract style per API surface
  - add targeted tests for:
    - task-create/start failure and idempotence paths
    - post-stop event handling
    - log adapter level/filter/format behavior

## Closure Criteria
- All High findings resolved or explicitly accepted with rationale.
- New/updated tests cover each remediated finding.
- `docs/ARCHITECTURE.md` and `docs/DEVLOG.md` updated to reflect final behavior.
- Findings register statuses moved from `Open` to `Done` (or `Accepted Risk`) with commit references.
