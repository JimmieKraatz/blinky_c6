# Development Log

## Why this exists
This file is a diary of development progress: what changed, why, and what is next.
For the stable technical view, see `docs/ARCHITECTURE.md`.

## Logging Conventions
- Slice naming: use numeric parent slices (`Slice 1`, `Slice 2`, ...) and lettered sub-slices (`Slice 5a`, `Slice 5b`, ...) when that makes the work easier to review.
- Module naming: prefer `app_` for app-plumbing contracts and `blinky_` for domain behavior; treat older `led_*` orchestrator names as legacy until refactored.
- For active feature lines, prefer a checklist slice summary with:
  - status checkbox
  - `Goal`
  - `Includes`
  - `Commit(s)`
  - `Sub-slices` when the parent slice spans multiple focused implementation steps

## 2026-03-09 - Repository structure and testing flow
### Context
The codebase started with a monolithic `components/blinky` module and local/manual
unit test execution through ESP-IDF unit-test-app.

### Changes
- Refactored into three modules:
  - `core_sm` for reusable state machine engines
  - `core_blinky` for reusable blinky domain logic
  - `blinky_idf` for ESP-IDF/hardware glue
- Moved tests to module ownership:
  - `core_sm/test/*`
  - `core_blinky/test/*`
  - `blinky_idf/test/*`
- Aligned README + VS Code tasks for unit-test-app build/flash/monitor workflow.
- Fixed unit-test `SDKCONFIG_DEFAULTS` layering and shell escaping in tasks.

### Notes
- Unity on target is menu-driven by design over serial (`*` to run all).
- CI without hardware should only build; CI with hardware can drive menu input.

## 2026-03-09 - Documentation split
### Context
`README.md` had both usage and architecture/process history mixed together.

### Changes
- Kept `README.md` focused on usage and commands.
- Added `docs/ARCHITECTURE.md` for stable technical structure.
- Converted this file into a dated process log.

## 2026-03-09 - Next milestone draft (event-driven transition)
### Intent
Transition runtime orchestration from polling/step-loop style to event-driven HSM.

### Why
- Align with `core_sm/hsm_engine.h` introduction.
- Separate event production (hardware/time) from state orchestration.
- Improve clarity and maintainability as behavior grows.

### Guardrails
- Preserve current user-visible behavior during migration.
- No dynamic allocation in runtime dispatch path (static queue only).
- Keep framework-specific primitives in `blinky_idf`.
- Keep `core_sm` and `core_blinky` framework-agnostic.

### Draft plan
1. Introduce adapter boundaries first (preparatory step, no behavior change):
   - input adapters (button now, others later)
   - output adapters (LED/render now, others later)
2. Route existing runtime calls through adapters while keeping current loop semantics.
3. Define event contract and ownership boundaries.
4. Add app-specific static queue + dispatcher in `blinky_idf` (no behavior change).
5. Route button/tick inputs through events.
6. Move orchestration transitions onto HSM.
7. Extend tests for event sequencing and transition behavior.
8. Remove old polling orchestration paths once parity is proven.

### Draft decisions
- One app-level dispatcher loop in `blinky_idf`.
- Static ring queue for `app_event_t` with overflow accounting.
- Adapterization is explicitly preparatory for event-driven HSM migration.
- `MENU` currently intended as child of `ACTIVE`.
- Menu exit target should return to prior state (`RUNNING` or `PAUSED`) via stored context.

### Open items
- Add HIL CI path for Unity execution (optional).
- Add diagrams for event/data flow (optional).

## 2026-03-09 - Adapter boundary extraction progress
### Context
Completed the preparatory boundary work for LED and button to reduce coupling before event/HSM migration.

### Changes
- Moved LED orchestration into core runtime and left ESP-IDF shell in `led_sm_idf.*`.
- Extracted button debouncer logic to `core_blinky/button_logic.*`.
- Renamed platform wrappers to explicit `_idf` names (`led_sm_idf.*`, `button_idf.*`).
- Created `components/blinky_interfaces/` and moved generic adapter contracts there:
  - `button_input_adapter.h`
  - `led_output_adapter.h`
- Expanded adapter/runtime tests for null-safety and menu behavior.

### Traceability (selected commits)
- `f0dc52a` move LED runtime orchestration into `core_blinky`
- `1768dca` rename `led_sm` shell to `led_sm_idf`
- `4c64527` extract core button logic
- `f6fefdd` rename button wrapper to `button_idf`
- `945e03c` move generic adapter interfaces to `blinky_interfaces`
- `bc5a9d2` add adapter null-safety/runtime behavior tests

### Notes
- Commit references are included here as lightweight breadcrumbs (no tags required).
- Stable architecture intent remains in `docs/ARCHITECTURE.md`; this file remains diary/process oriented.

## 2026-03-09 - Drafted app event contract list
### Context
After boundary extraction, the next step is formalizing event semantics and ownership.

### Changes
- Added a first-pass app event list to `docs/ARCHITECTURE.md`:
  - boot, tick, button short/long
  - optional/future events for menu timeout, model-step due, fault, shutdown
- Added explicit producer/consumer/payload notes per event.
- Added initial dispatcher ordering rules:
  - FIFO queue
  - insertion-order tie-break
  - no implicit priority unless explicitly defined later

### Notes
- This list is intentionally draft-level and may be reordered or trimmed without changing behavior contracts.

## 2026-03-10 - Drafted dispatcher shape and ownership
### Context
After defining the event list, the next prep step was clarifying where queueing/dispatch logic lives.

### Changes
- Added dispatcher shape section to `docs/ARCHITECTURE.md`:
  - producers in `blinky_idf`
  - static FIFO ring queue in `blinky_idf`
  - single dispatch path into core orchestration
- Added struct ownership rules:
  - shared portable event types
  - platform-owned queue implementation
  - core-owned event semantics/state transition handling
- Added boundary guardrails to prevent policy leakage across layers.

### Notes
- This is still design prep, not HSM implementation.

## 2026-03-10 - Dispatcher extracted to interfaces
### Context
After validating queue and consumer wiring, dispatcher mechanics were moved to the portable interfaces layer.

### Changes
- Added portable dispatcher contract and implementation to `components/blinky_interfaces/`:
  - `app_dispatcher.h`
  - `app_dispatcher.c`
- Added dispatcher unit tests in `components/blinky_interfaces/test/test_app_dispatcher.c`.
- Kept queue storage/mechanics in `blinky_idf` and rewired `led_sm_idf` to use `app_dispatcher`.

### Notes
- This keeps separation of concerns explicit:
  - interfaces own dispatch contracts
  - idf owns queue storage + hardware producers
  - core owns event semantics

## 2026-03-10 - Next implementation branch: async producer/consumer split
### Context
Event contract, queue, dispatcher, and core consumer extraction are merged and stable on `develop`.
Current execution is still parity mode (enqueue + immediate drain in same loop).

### Plan
- Create dedicated branch for async split.
- Keep producer path time-gated (`POLL_MS`) in `blinky_idf`.
- Move consumer dispatch to dedicated event-driven loop/task.
- Preserve core event semantics and adapter boundaries.
- Add queue high-water instrumentation before capacity tuning.

### Notes
- Queue capacity remains provisional during this slice and will be tuned from observed usage.

## 2026-03-10 - Async producer/consumer split implemented
### Context
Completed the async transition slice while preserving producer polling cadence and core behavior.

### Changes
- Split `led_sm_idf` orchestration into explicit producer and consumer units:
  - `led_sm_producer_idf.c`
  - `led_sm_consumer_idf.c`
- Kept `led_sm_idf.c` as IDF shell/orchestration entrypoint.
- Replaced app event queue internals with a static FreeRTOS queue backend (no dynamic allocation).
- Started a dedicated consumer task from `led_sm_init` and switched producer path to notify consumer on successful enqueue.
- Removed in-loop consumer draining from `led_sm_step` (producer remains `POLL_MS`-gated).
- Added targeted async wiring tests in `components/blinky_idf/test/test_led_sm_idf.c`:
  - notify safety before task start
  - producer notify path
  - burst drain on single notify

### Notes
- This closes the async consumer split goal while keeping producer timing unchanged.
- Next architectural work is lifecycle boundaries and overflow/backpressure policy.

## 2026-03-10 - VS Code unit-test task alignment
### Context
VS Code test tasks could omit `blinky_interfaces` tests, causing dispatcher tests not to appear in on-device menus.

### Changes
- Updated `.vscode/tasks.json` test tasks to include:
  - `-T blinky_interfaces`
- Aligned `IDF: Flash+Monitor (tests)` with `EXTRA_COMPONENT_DIRS` and `CCACHE_ENABLE=1`.

### Notes
- Escaping for `SDKCONFIG_DEFAULTS` remains `\\;` in JSON and validates correctly in task execution.

## 2026-03-10 - Core extraction slices 1-4 completed
### Context
Completed planned core/framework extraction slices for event wiring, startup policy,
wake ownership, and button timing policy ownership.

### Changes
- Slice 1: extracted core event construction policy (`led_event_factory.*`, later renamed to `app_event_factory.*`) and removed inline event defaults from `_idf`.
- Slice 2: extracted startup wave selection policy (`led_startup_policy.*`) behind core-facing config.
- Slice 3: moved wake/notify side-effects behind enqueue sink boundary; producer path now only publishes.
- Slice 4: introduced core-facing button timing contract (`button_policy.*`) and mapped `sdkconfig` through `_idf`.

### Verification
- Build validation: app + unit-test-app builds pass after each slice.
- On-target Unity run (user-reported):
  - `72 Tests 0 Failures 0 Ignored`

### Notes
- This branch is now merge-ready for the extraction scope.

## Deferred Items
Open deferred work is tracked in `docs/deferred_items_log.md`.
  - keep this separate from startup mode preference and persistence work
- Legacy orchestrator naming cleanup:
  - evaluate renaming `led_sm_idf.*` to `blinky_sm_idf.*` (or `app_sm_idf.*`) to match current architecture boundaries
  - perform as a dedicated refactor slice to avoid mixing with feature work

## Done TODOs
- Startup waveform ownership decoupling (completed 2026-03-10):
  - moved startup waveform semantics into core startup policy
  - removed startup waveform selection from Kconfig/framework surface
  - `_idf` now injects only core default startup selector input
  - completion commits: `57afe3a`, `e9d2fc0`, `f17870e`
- Logging boundary migration (completed 2026-03-11):
  - added portable structured logging contract in `blinky_interfaces` (`blinky_log.h`)
  - added ESP-IDF log adapter and routed `_idf` intensity logs through sink
  - migrated core runtime logging from direct `printf` to sink emission
  - restored parity/stability:
    - preserve init-time sink logs
    - fix consumer stack overflow under logging load
    - add newline termination for monitor readability
  - completion commits: `47fc9f4`, `8ec8f24`, `7369566`, `48c9283`
- Runtime behavior bug (menu exit apparent reset) resolved (2026-03-11):
  - root cause: consumer task stack overflow caused reboot/reset behavior
  - fix: increased consumer task stack and retained structured logging output parity
- Defaults/config ownership review follow-up completed (2026-03-11):
  - removed temporary startup-symbol compatibility path in mapper/config flow
  - completed ownership checklist pass after logging-boundary merge

## Active extraction roadmap
- Slice 1: extract core-owned wiring policy (completed)
- Slice 2: startup waveform policy to core-facing config input (completed)
- Slice 3: isolate notify/wake policy on consumer side only (completed)
- Slice 4: button timing policy ownership cleanup (completed)

## 2026-03-10 - Config/default ownership split kickoff
### Context
Started a dedicated branch to address the remaining config/default ownership ambiguity before logging boundary work.

### Changes
- Added an explicit config ownership decision table in `docs/ARCHITECTURE.md`.
- Identified `BLINKY_POLL_MS` as the primary mixed-ownership symbol to split next.

### Next implementation step
- Split `BLINKY_POLL_MS` into:
  - framework producer cadence knob (`_idf` loop delay)
  - core model cadence knob (`core_blinky` model config input)

## 2026-03-10 - Config ownership: poll cadence split
### Changes
- Split mixed `BLINKY_POLL_MS` into:
  - `BLINKY_PRODUCER_POLL_MS` (framework scheduler cadence)
  - `BLINKY_MODEL_POLL_MS` (core model cadence input)
- Updated `led_sm_idf` wiring so producer delay and model cadence are independently configured.
- Updated architecture docs/table to reflect the new ownership boundary.

### Why
- Removes coupling between task scheduling cadence and model behavior cadence.
- Makes future tuning safer (UI responsiveness vs model fidelity can be tuned independently).

## 2026-03-10 - Lifecycle boundary slice: explicit start/stop
### Changes
- Added explicit lifecycle APIs:
  - `led_sm_start(...)`
  - `led_sm_stop(...)`
  - `led_sm_consumer_task_stop(...)`
- Kept backward compatibility by having `led_sm_init(...)` call `led_sm_start(...)`.
- Added targeted lifecycle test:
  - `led sm stop lifecycle is idempotent`

### Test intent
- Verifies stop semantics are safe when called repeatedly.
- Guards against accidental event processing after consumer stop in async mode.

## 2026-03-10 - Logging boundary branch kickoff
### Branch
- `refactor/logging-boundary-plan`

### Intent
- Introduce a clean logging boundary so core logic does not depend on ESP-IDF logging APIs/macros.
- Keep behavior unchanged while clarifying ownership:
  - core emits log intents through interfaces/contracts
  - `_idf` decides sink, formatting, and output backend

### Scope guardrails
- In scope:
  - interface contract for logging
  - `_idf` logging adapter implementation
  - wiring/mapping updates where direct framework logging is currently mixed into non-framework paths
- Out of scope for this branch:
  - new product features
  - fault/shutdown behavior expansion
  - large test-framework redesign

### Planned slices
- Slice 1: inventory and classify current logging callsites by ownership (`core`, `interfaces`, `_idf`).
- Slice 2: add portable log contract in `blinky_interfaces` (levels + sink interface).
- Slice 3: add ESP-IDF adapter in `blinky_idf` and route existing `_idf` logging through adapter.
- Slice 4: migrate any core-adjacent callsites to boundary usage and remove direct framework leakage.
- Slice 5: tests + docs pass:
  - add/adjust unit tests for contract behavior and null-safe adapter behavior
  - run build/test validation and update architecture/devlog ownership notes

### Exit criteria
- No core-owned module includes or depends on ESP-IDF logging headers/macros.
- Logging behavior remains functionally equivalent at runtime.
- Ownership docs reflect final boundary and TODOs are updated (moved to done or deferred explicitly).

## 2026-03-10 - Logging boundary slice 1: callsite inventory
### Summary
- Inventory completed for runtime logging callsites.
- Current state uses `printf` (not `ESP_LOG*`) in both core and `_idf`.
- Highest-priority boundary leak: core runtime emits direct log output.

### Callsites by ownership
- `core_blinky`:
  - `components/core_blinky/led_runtime.c`:
    - `printf("STATE: RUNNING\\n")`
    - `printf("SINE_STEPS_USED=%u\\n", ...)`
    - `printf("STATE: PAUSED\\n")`
    - `printf("MODEL: %s\\n", ...)`
    - `printf("STATE: MENU\\n")`
    - `printf("MENU: WAVE %s\\n", ...)` (enter)
    - `printf("MENU: WAVE %s\\n", ...)` (change)
    - `printf("MENU: EXIT\\n")`
- `_idf`:
  - `components/blinky_idf/led_sm_idf.c`:
    - `printf("LED %u%%\\n", ...)` (gated by `log_intensity_enabled`)
- `blinky_interfaces`:
  - no logging callsites

### Config touchpoints relevant to logging
- `components/blinky_idf/Kconfig`: `BLINKY_LOG_INTENSITY`
- `components/blinky_idf/led_config_idf.*`: maps and carries `log_intensity_enabled`

### Implications for next slice
- Add portable logging contract in `blinky_interfaces`.
- Route core runtime logs through contract sink instead of direct `printf`.
- Keep `_idf` as the concrete sink owner (stdout/ESP log backend decision stays platform-owned).

## 2026-03-10 - Config ownership slice: mapper boundary introduced
### Changes
- Added explicit `_idf` mapper functions:
  - `idf_build_platform_config(...)`
  - `idf_build_core_config(...)`
- Added mapper-backed config contracts in `components/blinky_idf/led_config_idf.*`.
- Routed `led_sm_init(...)` and producer delay path through mapped config values with no behavior change.

### Why
- Keeps `sdkconfig` as source-of-truth in `_idf` while making ownership boundaries explicit.
- Establishes the handoff point where core-owned semantics can be fed by framework-sourced values.

## 2026-03-10 - Config ownership slice: button timing remapped to core config
### Changes
- Updated mapper contracts so button timing is no longer carried in platform config.
- `idf_build_core_config(...)` now owns normalized `button_timing` construction.
- `led_sm_init(...)` now composes `button_input_adapter_idf_config_t` from:
  - platform wiring (`gpio`, pull mode, active level)
  - core timing policy (`debounce_count`, `long_press_ms`)

### Why
- Keeps button semantic timing under core-owned policy while `_idf` remains the source adapter for `sdkconfig` values.

## 2026-03-10 - Config ownership slice: core config contract moved to core_blinky
### Changes
- Introduced core-owned config contract header:
  - `components/core_blinky/led_core_config.h`
- Updated `_idf` mapper to populate that core-defined contract instead of defining core config types in `_idf`.

### Why
- Clarifies ownership: `_idf` maps framework values, while core defines semantic config contracts.

## 2026-03-10 - Config ownership slice: startup wave selection tokenized
### Changes
- Reworked startup policy input from direct wave enum to a core-owned startup selector token.
- `_idf` now maps Kconfig start-wave choice to selector token values (not `led_wave_t` values).
- Core startup policy maps selector token to concrete `led_wave_t`.

### Why
- Reduces framework knowledge of domain waveform enumeration details.
- Keeps startup behavior semantics owned by core policy.

## 2026-03-10 - Config ownership slice: startup Kconfig decoupled from waveform names
### Changes
- Replaced wave-specific Kconfig choice (`BLINKY_START_WAVE_*`) with generic token:
  - `BLINKY_STARTUP_SELECTOR` (0..5)
- Updated `_idf` mapper to pass selector token directly to core startup policy input.

### Why
- Removes waveform-name coupling from Kconfig/framework configuration surface.
- Keeps framework as a value source, with startup interpretation remaining core-owned.

## 2026-03-10 - Config ownership slice: startup selection removed from Kconfig
### Changes
- Removed startup selection symbols from `components/blinky_idf/Kconfig`.
- Updated `_idf` mapper to inject `LED_STARTUP_SELECT_DEFAULT` only.
- Startup waveform choice is now purely core policy (default + future runtime source), not framework config.

### Why
- Fully removes startup-wave semantic configuration from framework/Kconfig.
- Keeps startup behavior ownership in core, with `_idf` only passing inputs and wiring outputs.
## 2026-03-11 - Critical review branch kickoff
### Branch
- `review/findings-hardening-2026-03-11`

### Context
Requested a full critical review pass before further feature work.

### Changes
- Added critical findings register and remediation plan:
  - `docs/reviews/CRITICAL_REVIEW_2026-03-11.md`
- Captured seven findings across lifecycle, startup ordering, config hardening, and test depth.
- Defined remediation in four focused slices with explicit closure criteria.

### Notes
- High-severity lifecycle/startup issues are prioritized ahead of new feature additions.

## 2026-03-11 - Critical review slice 1: non-singleton lifecycle + start modes
### Context
Addressed first hardening slice for lifecycle ownership and explicit restart semantics.

### Changes
- Converted consumer task ownership from file-static singleton to per-context fields on `sm_led_ctx_t`.
- Added explicit start modes:
  - `LED_SM_START_FRESH`
  - `LED_SM_START_RESUME`
- Updated `led_sm_start(...)` API to accept lifecycle mode and return success/failure.
- `led_sm_init(...)` now starts using explicit fresh mode.
- Added targeted lifecycle tests in `test_led_sm_idf.c`:
  - fresh start resets runtime and emits boot event
  - resume start preserves runtime and does not emit boot event
  - start idempotence check on resume path

### Verification
- Unit-test-app build passes with targets:
  - `core_sm`, `core_blinky`, `blinky_idf`, `blinky_interfaces`

## 2026-03-11 - Critical review slice 2: startup ordering race hardening
### Context
Addressed startup ordering race between boot-pattern LED writes and async consumer output path.

### Changes
- Reordered fresh-start flow in `led_sm_start(...)`:
  - run runtime fresh init
  - run boot pattern
  - start consumer task
  - enqueue boot event
- Removed duplicate runtime reinit in `led_sm_init(...)` so fresh-start owns initialization sequencing.

### Why
- Prevents consumer-driven LED output from interleaving with synchronous startup pattern writes.

### Verification
- Unit-test-app build passes with targets:
  - `core_sm`, `core_blinky`, `blinky_idf`, `blinky_interfaces`
- On-device Unity run checkpoint:
  - `82 Tests 0 Failures 0 Ignored`

## 2026-03-11 - Critical review slice 3a: platform timing normalization
### Context
Addressed config normalization risk for platform-owned timing values.

### Changes
- Added mapper-side normalization in `idf_build_platform_config(...)`:
  - `BLINKY_PRODUCER_POLL_MS` -> minimum `1 ms`
  - `BLINKY_BOOT_PATTERN_MS` -> minimum `1 ms`
- Added Kconfig UI guardrails:
  - `BLINKY_PRODUCER_POLL_MS` range: `1..1000`
  - `BLINKY_BOOT_PATTERN_MS` range: `1..2000`
- Added targeted mapper clamp tests:
  - `components/blinky_idf/test/test_led_config_idf.c`
  - validates min clamp, max clamp, and in-range pass-through behavior

### Why
- Keeps normalization at boundary (not call-sites), consistent with ownership rules.
- Prevents zero/invalid delays from causing tight-loop behavior or collapsed boot timing.

## 2026-03-11 - Critical review slice 4 decision: null-contract policy
### Context
CR-006 identified inconsistent null-safety behavior across module surfaces and mixed strict/defensive handling in some core APIs.

### Decision
- This is an architectural decision at module-boundary level, not a local style preference.
- `core_*` APIs will use strict/fail-fast contracts for required pointers and invariants.
- `blinky_interfaces` and `blinky_idf` APIs will remain defensive and return status on invalid arguments/state.
- Optional pointers must be documented explicitly in headers.
- Lifecycle APIs should validate state transitions, not only pointer presence.

### Next implementation targets
- Normalize `led_runtime_*` contract behavior to remove mixed partial null handling.
- Add/update header contract notes (`required` vs `optional`) on affected APIs.
- Add targeted tests for CR-007 lifecycle and log-adapter depth items.

## 2026-03-11 - Critical review slice 4 implementation: contract alignment + test hardening
### Summary
- Implemented the slice 4 policy decision and closed CR-006/CR-007 implementation scope.

### Changes
- Core contract alignment:
  - enforced strict required-pointer assertions in core runtime/model/policy/menu/button/consumer APIs
  - removed mixed partial-null handling patterns in core APIs
  - documented required vs optional pointer contracts in core headers
- Async lifecycle test hardening (`test_led_sm_idf.c`):
  - added start failure path test by forcing consumer task create to return null
  - added post-stop enqueue behavior test (no dispatch until resumed + notified)
- Log adapter test hardening (`test_blinky_log_adapter_idf.c`):
  - added min-level filtering test
  - added structured formatting test for domain/event/message + typed key-values
  - added log-write test seam to capture emitted line/tag/level in unit tests

### Verification
- Unit-test-app build passes with targets:
  - `core_blinky`, `blinky_idf`, `blinky_interfaces`
- command used:
  - `idf.py -C $IDF_PATH/tools/unit-test-app -B $PWD/build/unit-test-app -D EXTRA_COMPONENT_DIRS=$PWD/components -D SDKCONFIG=$PWD/build/unit-test-app/sdkconfig -D "SDKCONFIG_DEFAULTS=$IDF_PATH/tools/unit-test-app/sdkconfig.defaults;$PWD/test/unit-test-app.sdkconfig.defaults" -D CCACHE_ENABLE=1 -T core_blinky -T blinky_idf -T blinky_interfaces build`

### Commit
- `643f9bf`

## 2026-03-11 - Logging boundary slice 2: portable contract added
### Summary
- Added a portable structured logging contract in `blinky_interfaces`:
  - `components/blinky_interfaces/blinky_log.h`
- Added interface-focused tests:
  - `components/blinky_interfaces/test/test_blinky_log.c`

### Contract shape
- Record fields:
  - `level`, `domain`, `event`, `message`
  - optional typed key-value payload (`kvs`, `kv_count`)
- Sink boundary:
  - `blinky_log_sink_ops_t.emit(...)`
  - `blinky_log_emit(...)` inline helper with null-safe behavior
- Value typing:
  - `INT`, `UINT`, `BOOL`, `STR` via `blinky_log_kv_type_t`
  - helper constructors (`blinky_log_kv_int/uint/bool/str`)

### Why this shape
- Avoids heap allocation and varargs formatting in core paths.
- Keeps core output semantic/structured while leaving rendering/backend to `_idf`.
- Improves testability by asserting record fields instead of formatted strings.

### Verification
- Unit-test-app build succeeds and includes `test_blinky_log.c`.

## 2026-03-11 - Logging boundary slice 3: ESP-IDF adapter + _idf routing
### Summary
- Added concrete ESP-IDF logging adapter:
  - `components/blinky_idf/blinky_log_adapter_idf.h`
  - `components/blinky_idf/blinky_log_adapter_idf.c`
- Added `_idf` adapter tests:
  - `components/blinky_idf/test/test_blinky_log_adapter_idf.c`
- Routed `_idf` intensity logging path in `led_sm_idf.c` through `blinky_log_emit(...)` instead of direct `printf`.

### Wiring changes
- `sm_led_ctx_t` now owns:
  - `blinky_log_sink_t log_sink`
  - `blinky_log_adapter_idf_t log_idf`
- `led_sm_init(...)` initializes adapter with tag `"blinky"` and `INFO` minimum level.
- `BLINKY_LOG_INTENSITY` behavior is preserved (same gate, new sink path).

### Verification
- Unit-test-app build passes with adapter and new tests included.

## 2026-03-11 - Logging boundary slice 4: core runtime callsites migrated
### Summary
- Migrated core runtime direct logging from `printf` to structured sink emission.
- `led_runtime` now supports optional sink injection:
  - `led_runtime_set_log_sink(led_runtime_t *rt, blinky_log_sink_t *sink)`
- `_idf` wiring now injects the sink into core runtime after init.

### Callsite status
- Removed direct `printf` usage from:
  - `components/core_blinky/led_runtime.c`
- Core runtime now emits structured records for:
  - state transitions (`running`, `paused`, `menu`)
  - menu wave changes
  - menu exit

### Tests and verification
- Added runtime test covering structured log emission when sink is configured.
- Unit-test-app build passes with updated API and tests.

### Follow-up parity fix
- Preserved preconfigured runtime sink across `led_runtime_init(...)` so init-time state logs are not dropped.
- Updated `_idf` init flow to set runtime sink before `led_runtime_init(...)`.
- Added test coverage for init-time log emission with preconfigured sink.
- Verified on-device behavior after follow-up fixes:
  - menu exit no longer triggers startup-like reset sequence
  - line-based logging output restored in monitor

## 2026-03-11 - Logging boundary slice 5: configurability and boot identity
### Summary
- Added framework-owned log verbosity control in Kconfig:
  - `BLINKY_LOG_MIN_LEVEL_{ERROR,WARN,INFO,DEBUG}`
- Mapped selected level through `_idf` config mapper into platform config.
- Routed sink init to use mapped `log_min_level` instead of hardcoded value.
- Added structured app boot identity log record emitted at startup:
  - domain/event: `app:boot`
  - message: `startup`
  - keys: `project`, `version`, `idf`, `min_level`

### Wiring detail
- Added `esp_app_format` component dependency to `blinky_idf` for app description metadata.
- Boot identity record is emitted immediately after sink adapter init in `led_sm_init(...)`.

### Verification
- App build passes.
- Unit-test-app build passes.

## 2026-03-12 - CI/CD implementation plan (sliced)
### Context
Delivery policy has been expanded in `docs/DELIVERY_WORKFLOW.md`. Next step is implementation with low-risk slices so we get fast feedback without blocking on HIL infrastructure.

### Scope guardrails
- Keep first rollout cloud-only for hardware-independent checks.
- Treat HIL as a staged/manual gate until self-hosted runner is stable.
- Keep each slice shippable and reviewable in a single focused PR when possible.

### Implementation slices
1. Slice 1 - Workflow scaffolding
   - Add `.github/workflows/ci.yml` with trigger skeleton only.
   - Add `.github/workflows/release.yml` with tag trigger skeleton only.
   - Add `.github/workflows/hil-smoke.yml` as `workflow_dispatch` + `self-hosted` stub.
   - Acceptance: workflows parse and appear in GitHub Actions without running risky jobs.

2. Slice 2 - Cloud app build gate
   - Implement ESP-IDF setup on `ubuntu-latest`.
   - Add `idf.py set-target esp32c6` and `idf.py -D CCACHE_ENABLE=1 build`.
   - Wire to `pull_request` and `push` on `develop`/`master`.
   - Acceptance: PRs show a required app build check.

3. Slice 3 - Unit-test-app build gate
   - Add unit-test-app build command with current component targets:
     - `core_sm`, `core_blinky`, `blinky_idf`, `blinky_interfaces`
   - Keep this as compile/integration validation (no on-device execution in cloud).
   - Acceptance: PRs show separate required test-build check.

4. Slice 4 - Release automation
   - On tag push `v*`, build release artifacts and publish GitHub Release.
   - Attach binaries/checksums and generated release notes.
   - Mark `-rc` tags as pre-release.
   - Acceptance: creating `vX.Y.Z` or `vX.Y.Z-rc.N` produces correct release output.

5. Slice 5 - HIL manual smoke path
   - Implement self-hosted flash + serial startup smoke script.
   - Upload serial logs on failure.
   - Keep trigger manual (`workflow_dispatch`) and non-required.
   - Acceptance: one successful end-to-end manual HIL run is recorded.

6. Slice 6 - Branch protections and policy enforcement
   - Make cloud checks required on `develop` and `master`.
   - Keep HIL optional until flake rate is acceptable.
   - Add README badges/links to workflow status and policy doc.
   - Acceptance: merge blocked when required checks fail.

### Out of scope for initial rollout
- Full scripted interaction testing of menu/button behavior in cloud.
- Mandatory HIL gate before runner reliability is proven.
- Packaging/deploy integration beyond GitHub Releases.

## 2026-03-12 - CI/CD slice 1 implemented: workflow scaffolding
### Changes
- Added workflow scaffold files:
  - `.github/workflows/ci.yml`
  - `.github/workflows/release.yml`
  - `.github/workflows/hil-smoke.yml`
- Added safe placeholder jobs only (no ESP-IDF setup, no flash, no artifact publishing yet).
- Configured triggers:
  - `ci.yml`: `pull_request` + `push` on `develop` and `master`
  - `release.yml`: `push` on tags matching `v*`
  - `hil-smoke.yml`: `workflow_dispatch` only

### Notes
- HIL workflow is intentionally disabled (`if: false`) while self-hosted runner infrastructure is being prepared.
- Next slice is cloud app build gate (`idf.py set-target esp32c6` + `idf.py build`).

## 2026-03-12 - CI/CD slice 2 implemented: cloud app build gate
### Changes
- Updated `.github/workflows/ci.yml` to replace placeholder scaffold job with an app build job.
- Added checkout step (`actions/checkout@v4`) with submodules enabled.
- Added ESP-IDF build step via `espressif/esp-idf-ci-action@v1`:
  - `idf.py set-target esp32c6`
  - `idf.py -D CCACHE_ENABLE=1 build`

### Notes
- Trigger scope remains:
  - `pull_request` on `develop`, `master`
  - `push` on `develop`, `master`
- Slice 3 will add a separate unit-test-app build gate.

## 2026-03-12 - CI/CD slice 2 follow-up: first-run failure remediation
### Context
First GitHub Actions run failed in `blinky_idf` on missing `driver/gpio.h` dependency resolution under newer ESP-IDF layout.

### Changes
- Updated CI workflow hardening:
  - `actions/checkout@v5`
  - pinned ESP-IDF version in CI action (`v5.5.2`) to match local development baseline and avoid unplanned `-dev` toolchain drift.
- Reverted attempted `esp_driver_gpio` requirement after validation showed it is unavailable in pinned IDF line used by CI gate.

### Notes
- Kept `driver` requirement for Slice 2 compatibility under pinned CI toolchain.
- IDF 6 component migration can be handled later as a dedicated compatibility slice.

## 2026-03-13 - CLI naming alignment pass
### Context
As CLI control-plane scaffolding was added, naming drift appeared between older `led_*` mapping/factory terms and app-layer event plumbing.

### Changes
- Renamed `led_event_factory.*` -> `app_event_factory.*`.
- Renamed `led_cli_command_map.*` -> `app_cli_command_map.*`.
- Updated references and tests to use new app-layer naming consistently.

### Notes
- `blinky_cli_command_t` remains intentionally domain-scoped (`blinky_`), while mapping/factory modules stay app-plumbing scoped (`app_`).

### Commit(s)
- `e106714`, `8b90e69`, `ec2a923`

## 2026-03-13 - CLI control-plane kickoff (planned v0.2.0)
### Context
Next feature direction is a user-facing CLI that mirrors button-driven behavior and provides a reusable control surface for future transport/provisioning paths (for example BT mesh/Wi-Fi) and platform portability (ESP32 + nRF52).

### Version intent
- Target release for this feature line: `v0.2.0` (new observable feature surface).

### Scope guardrails
- Keep architecture split aligned with existing boundaries:
  - core: parsing-independent command model + behavior mapping contracts
  - platform (`_idf`): console/UART transport and adapter wiring
- CLI v1 should first mirror existing button/menu behavior before adding broader config commands.
- NVS-backed settings are planned as follow-on within CLI feature track (not required for first command loop).
- Future control surfaces are likely to extend beyond local serial CLI:
  - possible wireless provisioning path
  - possible MQTT control path
  - possible mesh-oriented command/control path
- Slice 4 planning must account for that expansion so CLI/config work does not overfit a UART-only or "virtual button only" model.

### Slice summary
- [x] Slice 1: CLI contracts and event-mapping foundation
  - Goal: define the portable command contract and map CLI intent into the app event path without tying the surface to ESP-IDF transport details.
  - Includes: `blinky_interfaces` command contract, core command mapping, CLI event-factory path, and the naming alignment from legacy `led_*` app-plumbing names to `app_*`.
  - Commit(s): `feaa3c6`, `e106714`, `8b90e69`
- [x] Slice 2: IDF CLI adapter
  - Goal: add the first concrete ESP-IDF transport adapter so CLI text input can enter the existing app event flow.
  - Includes: UART0 line-reader adapter wiring in `_idf`, non-blocking reads, parser bridge hookup, and Kconfig controls for CLI enablement and RX buffer sizing.
  - Commit(s): `28e5cfb`
- [x] Slice 3: CLI command-set parity and console bring-up
  - Goal: reach useful v1 parity with button/menu behavior and make the console path reliable enough for repeatable on-device smoke validation.
  - Includes: parser extraction into `core_blinky`, command vocabulary for `run`/`pause`/menu/status/help, smoke helper tooling, USB Serial/JTAG console support, input echo/edit behavior, and runtime-state gating correction for explicit menu commands.
  - Commit(s): `64cb69e`, `5fd5ee1`, `17e4e19`, `6809c24`, `2ad0ba6`, `59aaca2`, `60ba9e9`, `b9ec755`, `9e36dc8`
- [x] Slice 4: CLI/persistence foundation
  - Goal: define a transport-friendly split between runtime-control commands and persistence/config concerns, then scaffold the first persistence path behind explicit storage contracts.
  - Includes: command-intent routing decision, app settings contract, `_idf` NVS-backed store scaffold, and the first migrated persisted settings for startup and logging preferences.
  - Commit(s): `9a3437d`, `59d5a94`, `845a020`, `97ce003`, `8f1fc19`, `b85d5e6`, `13810d9`
  - Sub-slices:
    - [x] Slice 4a: command-intent path decision and LED-domain routing cleanup
      - Notes: CLI now routes explicit blinky command intent and the LED domain owns current-state acceptance/ignore behavior instead of forcing CLI to masquerade as raw button input.
      - Commit(s): `9a3437d`, `59d5a94`, `845a020`
    - [x] Slice 4b: persistence contract scaffold
      - Notes: introduced `app_settings_t` plus the storage boundary contract (`load` / `save` / `reset`) with scaffold-first payload/default/validation coverage.
      - Commit(s): `97ce003`
    - [x] Slice 4c: `_idf` settings store scaffold and first migrated settings
      - Notes: added NVS-backed store plumbing in `_idf`, dedicated app-owned partition usage, and first-pass persisted settings for `boot_pattern`, `log_intensity`, `log_level`, and `startup_wave`.
      - Commit(s): `8f1fc19`, `b85d5e6`
    - [x] Slice 4d: CLI config commands
      - Goal: expose a minimal config command surface for persisted startup/logging preferences without forcing config operations through the LED runtime command lane.
      - Planned items:
        - define the config-command contract and parser path separate from blinky runtime commands
          - [x] top-level help surface:
            - `help`
            - `help config`
          - [x] config read commands:
            - `config show`
            - `config show startup`
            - `config show logging`
          - [x] config write commands:
            - `config startup wave <square|saw_up|saw_down|triangle|sine>`
            - `config boot-pattern <on|off>`
            - `config log intensity <on|off>`
            - `config log level <error|warn|info|debug>`
          - [x] persistence/lifecycle commands:
            - `config save`
            - `config reset`
        - completed: implement the top-level help/config-help surface so the new config command family is discoverable from the existing CLI entrypoint
        - completed: implement the read path for the contracted `config show` commands, including focused `startup` and `logging` views if that stays proportionate
        - completed: implement the first write path for the contracted startup/logging setters, keeping the initial landed surface intentionally small if needed
        - completed: implement `config save` against the settings-store-backed persistence path
        - completed: implement `config reset` so persisted settings return to default-backed values cleanly
        - completed: add unit coverage for config parse behavior, command handling/dispatch, and targeted persistence-path validation
      - Notes: keep startup mode out of this slice until it has a concrete modeled representation; do not introduce a generic key/value grammar or broad hardware/timing/perf config surface here.
      - Validation follow-through landed in Slice 4e without requiring an artificial extra commit boundary.
      - Commit(s): `13810d9`
    - [x] Slice 4e: persistence validation and HIL follow-through
      - Notes: completed merge/default-rule coverage, adapter validation, and on-device config-command validation for read/write/save/reset behavior with observed NVS writes.
      - Validation:
        - targeted unit-test-app run: `127 / 0 / 0`
        - on-device app validation: `help`, `help config`, `config show`, `config show startup`, `config show logging`, setter commands, `config save`, and `config reset` behaved as expected
      - Commit(s): `13810d9`

### Later follow-through
- The docs/release closeout work for this feature line continues in the newer `2026-03-27` checklist entry below so the active work stays near the bottom of the journal.

### Slice 4A draft decision memo
#### Problem statement
Persistence/config commands are not a clean fit for the current "CLI as named button/control adapter" path. Runtime-control commands such as `run`, `pause`, `menu enter`, `menu next`, and `menu exit` map reasonably onto existing runtime semantics, but commands such as `config save`, `config reset`, provisioning actions, or future network-driven control do not.

#### Options considered
1. Keep all CLI intents flowing through the existing runtime/button-style event path.
   - Pros: minimal new plumbing
   - Cons: overfits semantic/config intents onto runtime events; likely to get awkward for save/reset/provisioning
2. Keep one physical CLI adapter, but split command handling into two intent paths.
   - runtime-control commands -> existing app/runtime event path
   - config/operational commands -> app-shell / `_idf` command path with storage boundary
   - Pros: preserves current runtime behavior model while creating room for persistence and future transports
   - Cons: introduces a second command-handling lane that must be documented clearly
3. Add a dedicated CLI state machine now.
   - Pros: maximal flexibility for future multi-step command workflows
   - Cons: likely premature for current scope; adds complexity before command/config boundary decisions are settled

#### Current recommendation
- Choose Option 2, but refine it further:
  - CLI/router should classify command domain, not own LED behavior mapping
  - `led_sm` should remain the owner of blinky runtime behavior/state transitions
  - runtime-control commands should be forwarded to the LED domain as explicit blinky commands, not as disguised button events
  - persistence/config/provisioning intents should not be forced through the same button-style event model
  - persistence/storage orchestration should live in app-shell / `_idf` boundaries, with core defining contracts where semantics belong in core

#### Why this recommendation fits future work
- Leaves room for serial CLI today without baking in UART-only assumptions.
- Scales better if provisioning is later exposed through CLI, wireless setup flows, MQTT, or mesh control.
- Avoids teaching the runtime state machine about storage and operational concerns that are not inherently waveform/menu behavior.

#### Refined command-intent path for Slice 4A
- Keep LED runtime states small and behavioral:
  - `running`
  - `paused`
  - `menu`
- Do not expand LED states into command names such as `run`, `pause`, `menu enter`, `menu next`, or `menu exit`.
- Introduce explicit blinky command intent separate from button input semantics:
  - `RUN`
  - `PAUSE`
  - `MENU_ENTER`
  - `MENU_NEXT`
  - `MENU_EXIT`
- Let the LED domain interpret those commands against current runtime state and decide whether they are:
  - applied
  - ignored
  - invalid
- Treat button input and CLI as separate producers of LED-domain intent rather than forcing CLI to masquerade as raw button input.

#### Separation-of-concerns rule
- CLI/router decides only:
  - which domain a command belongs to (`blinky`, `config`, `diagnostic`, future provisioning/network)
  - whether the command should be routed to a domain owner
- LED domain decides:
  - what a blinky command means in the current LED/menu state
  - whether that command is appropriate right now
  - what internal transition/action should result
- Non-blinky commands should route to non-LED owners and should not require `led_sm` awareness.

#### Planned output of Slice 4A
- An explicit architecture note describing the command-intent split:
  - domain routing in CLI/app shell
  - LED-domain command interpretation in `led_sm` or adjacent LED-domain controller
- A persistence/config contract candidate for Slice 4B.
- A small initial config-command surface proposal for Slice 4D.

#### Slice 4A implementation checklist

| Status | Item |
|---|---|
| Done | Refactor current CLI flow without adding any new commands. |
| Done | Preserve the existing user-visible command set: `help`, `status`, `run`, `pause`, `menu enter`, `menu next`, `menu exit`. |
| Done | Introduce an explicit blinky command intent contract separate from button/raw blinky events. |
| Done | Update CLI/app-shell routing so CLI identifies blinky-domain commands without mapping them directly to button events. |
| Done | Update LED-domain handling so blinky commands are interpreted by the LED domain against current runtime state (`running`, `paused`, `menu`). |
| Done | Keep button input as a separate producer path; do not force CLI to masquerade as raw button input. |
| Done | Update parser/command tests to reflect command-intent routing rather than direct CLI-to-button mapping assumptions. |
| Done | Add LED-domain command interpretation tests for current-state handling. |
| Done | Preserve negative-path coverage for invalid command/state combinations. |
| Done | Re-run app build and unit-test-app build after the refactor. |
| Done | Re-run on-device CLI smoke validation after the refactor. |

#### Persistence/config inventory candidate for Slice 4B
Notes:
- Distinguish live runtime state from persisted user preference.
- A possible persisted setting is instead startup behavior preference, for example:
  - start in `running`
  - start in `paused`
- Likewise, "pause behavior" is a separate policy question:
  - whether paused forces LED off
  - or freezes current brightness
  - that should not be conflated with startup mode or current runtime state

| Item / symbol | Current source | Current owner/boundary | Persistence candidacy | Notes |
|---|---|---|---|---|
| `BLINKY_LED_GPIO` | Kconfig / `sdkconfig` | `_idf` platform wiring | stay in `Kconfig` | Board/hardware wiring |
| `BLINKY_BTN_GPIO` | Kconfig / `sdkconfig` | `_idf` platform wiring | stay in `Kconfig` | Board/hardware wiring |
| `BLINKY_BTN_ACTIVE_LOW` | Kconfig / `sdkconfig` | `_idf` electrical interface | stay in `Kconfig` | Hardware/electrical behavior |
| `BLINKY_BTN_PULL_*` | Kconfig / `sdkconfig` | `_idf` electrical interface | stay in `Kconfig` | Hardware/electrical behavior |
| `BLINKY_PWM_FREQ_HZ` | Kconfig / `sdkconfig` | `_idf` LEDC/platform setup | stay in `Kconfig` | Peripheral setup |
| `BLINKY_PRODUCER_POLL_MS` | Kconfig / `sdkconfig` | `_idf` scheduler cadence | stay in `Kconfig` | Platform/task timing |
| `BLINKY_CLI_ENABLE` | Kconfig / `sdkconfig` | `_idf` transport wiring | stay in `Kconfig` | Build/deployment choice |
| `BLINKY_CLI_UART_RX_BUF_SIZE` | Kconfig / `sdkconfig` | `_idf` transport wiring | stay in `Kconfig` | Build/runtime buffer sizing, not user-facing |
| `BLINKY_BOOT_PATTERN` | Kconfig / `sdkconfig` | `_idf` platform/UI behavior | first-pass NVS candidate | User-facing startup preference |
| `BLINKY_BOOT_PATTERN_MS` | Kconfig / `sdkconfig` | `_idf` platform/UI behavior | maybe NVS later | Likely keep build-time initially |
| `BLINKY_LOG_INTENSITY` | Kconfig / `sdkconfig` | `_idf` logging policy | first-pass NVS candidate | Operational preference |
| `BLINKY_LOG_MIN_LEVEL_*` | Kconfig / `sdkconfig` | `_idf` logging policy | first-pass NVS candidate | Operational preference |
| `BLINKY_WAVE_PERIOD_MS` | Kconfig -> core config | `core_blinky` model policy | maybe NVS | User-facing if waveform tuning becomes a feature |
| `BLINKY_MODEL_POLL_MS` | Kconfig -> core config | `core_blinky` model cadence | maybe NVS later | More advanced tuning knob; not first-pass |
| `BLINKY_SINE_STEPS_MAX` | Kconfig -> core config | `core_blinky` quality/perf policy | maybe NVS later | Advanced tuning; probably not first-pass |
| `BLINKY_SAW_STEP_PCT` | Kconfig -> core config | `core_blinky` waveform shape policy | maybe NVS later | Advanced tuning; probably not first-pass |
| `BLINKY_DEBOUNCE_COUNT` | Kconfig -> core config | `core_blinky` button timing policy | stay in `Kconfig` | Not a user-facing operational preference |
| `BLINKY_LONG_PRESS_MS` | Kconfig -> core config | `core_blinky` button timing policy | stay in `Kconfig` | Not a user-facing operational preference |
| Startup wave preference | Core default today | `core_blinky` startup policy | first-pass NVS candidate | Strong user-facing preference |
| Startup mode preference (`running` vs `paused`) | Not yet modeled | LED-domain startup policy | first-pass NVS candidate | Separate from live runtime state |
| Pause output behavior (`LED off` vs `freeze brightness`) | Deferred policy decision | LED-domain pause policy | decide before NVS | Policy choice, not current-state persistence |

#### Provisional first-pass NVS surface
- `startup wave preference`
- `startup mode preference` (`running` vs `paused`)
- `boot pattern`
- `log intensity`
- `log min level`

#### Slice 4B scaffold decision
Before moving any real config items into persistence, scaffold the boundary with simple test data:
- core owns the persisted payload shape and default/validation rules
- interfaces own the storage contract (`load` / `save` / `reset`)
- `_idf` will later own the NVS-backed implementation

Initial `_idf` NVS backing direction:
- dedicated app-owned partition
- one namespace
- unencrypted for now
- simple typed keys
- add more partitions later only when reset/security/lifecycle needs diverge

Initial placeholder payload:
- `schema_version`
- `boot_pattern_enabled`
- `test_counter`
- `test_mode_enabled`

Why this comes first:
- validates the persistence architecture without coupling early mistakes to real user-facing config
- keeps Slice 4B focused on storage correctness before policy migration
- gives us a schema/version foothold for future migration handling

#### Slice 4B first migrated setting
- `boot_pattern_enabled`, `log_intensity_enabled`, `log_min_level`, and `startup_selector` are the first real settings moved onto the app settings boundary.
- Effective defaults still come from the existing `_idf` config path plus current core startup default (`BLINKY_BOOT_PATTERN`, `BLINKY_LOG_INTENSITY`, `BLINKY_LOG_MIN_LEVEL_*`, and `led_core_config.startup.selector`).
- If no persisted settings exist yet, startup seeds the settings store from that current default layer.
- If persisted settings exist, startup applies the stored settings overrides before the boot pattern is shown and before runtime logging behavior begins.
- `BLINKY_BOOT_PATTERN_MS` remains Kconfig-backed for now.
- startup wave now persists through `startup_wave`
- startup mode is still deferred until it has a concrete modeled representation

#### Slice 4B migrated key set
- `schema_ver`
- `boot_pattern`
- `log_intensity`
- `log_level`
- `startup_wave`
- `test_count`
- `test_mode`

#### Slice 4B schema note
- Added [docs/PERSISTENCE_SCHEMA.md](/workspaces/blinky_c6/docs/PERSISTENCE_SCHEMA.md) as the working schema/version reference for NVS-backed settings.
- Current behavior on missing or invalid persisted data is to reseed from current defaults.

#### Explicit non-goals for first-pass NVS
- hardware wiring/electrical values
- task/transport buffer sizing
- debounce/long-press timing
- broad waveform tuning/perf knobs unless a user-facing requirement emerges

#### Small initial config-command surface proposal for Slice 4D
Goals:
- Keep the first config surface small, explicit, and easy to validate on-device.
- Prefer commands that expose the first-pass NVS items without opening a broad generic settings language yet.

Proposed read-only commands:
- `config show`
  - dumps persisted/effective config summary
- `config show startup`
  - shows startup wave, startup mode, and boot-pattern enable
- `config show logging`
  - shows log intensity and min log level

Proposed mutating commands:
- `config startup wave <square|saw_up|saw_down|triangle|sine>`
- `config startup mode <running|paused>`
- `config boot-pattern <on|off>`
- `config log intensity <on|off>`
- `config log level <error|warn|info|debug>`

Proposed persistence/lifecycle commands:
- `config save`
- `config reset`

Deferred from initial config command surface:
- free-form key/value command grammar
- direct editing of timing/performance knobs
- GPIO/electrical/platform transport settings
- provisioning/network commands

Command-surface note:
- `config reset` should reset persisted config/preferences, not live runtime state.
- `config show` should present both effective values and, when useful, whether they came from defaults or persisted overrides.

### Commit(s)
- `6092009`, `9a3437d`, `59d5a94`, `845a020`, `97ce003`, `8f1fc19`, `b85d5e6`

## 2026-03-13 - CI/CD slice 2 follow-up: app_main link failure remediation
### Context
CI app build failed at link stage with `undefined reference to app_main`.

### Changes
- Removed conditional compilation guard around `app_main` in `main/main.c`.
- `app_main` is now always compiled for this application target.

### Notes
- This avoids configuration-dependent omission of the entry symbol in cloud builds.

## 2026-03-13 - CI/CD slice 3 implemented: unit-test-app build gate
### Changes
- Extended `.github/workflows/ci.yml` with a separate `unit-test-app-build` job.
- Added ESP-IDF unit-test-app build commands in CI for component integration validation:
  - `core_sm`
  - `core_blinky`
  - `blinky_idf`
  - `blinky_interfaces`
- Kept cloud scope build-only (no on-device Unity execution).

### Notes
- CI now reports two cloud gates on PR/push:
  - `App Build (ESP-IDF)`
  - `Unit Test App Build (ESP-IDF)`

## 2026-03-13 - CI/CD slice 4 implemented: release automation
### Changes
- Replaced `.github/workflows/release.yml` scaffold with full tag-driven release workflow.
- Added artifact build job on `push` tags `v*`:
  - app build with pinned ESP-IDF `v5.5.2`
  - collected release files:
    - `blinky_c6.bin`
    - `blinky_c6.elf`
    - `bootloader.bin`
    - `partition-table.bin`
    - `flasher_args.json`
    - optional `blinky_c6.map`
  - generated `sha256sums.txt`
- Added publish job:
  - downloads artifacts from prior job
  - creates GitHub Release with generated release notes
  - marks tags containing `-rc.` as pre-release

### Notes
- Release workflow is tag-driven by design; verification requires pushing a test tag.

## 2026-03-13 - CI/CD slice 4 validation: end-to-end release test
### Changes
- Pushed temporary validation tag: `v0.1.1-rc.0-test` (from `develop` head).
- Confirmed release workflow execution and artifact publication.
- Verified 9 release assets were generated (firmware, bootloader, partition table, metadata, checksums, and source archives).
- Removed temporary release tag locally and on origin after validation.

### Notes
- Slice 4 acceptance criteria satisfied: tag-driven artifact build and release publishing works in GitHub Actions.

## 2026-03-13 - CI/CD slice 5 implemented: manual self-hosted HIL smoke path
### Changes
- Replaced disabled HIL scaffold with executable manual workflow in `.github/workflows/hil-smoke.yml`.
- Added `workflow_dispatch` inputs:
  - `serial_port`
  - `startup_pattern`
  - `monitor_timeout_seconds`
- Added self-hosted smoke sequence:
  - checkout
  - preflight `idf.py` check
  - build (`idf.py set-target esp32c6 build`)
  - flash (`idf.py -p <port> flash`)
  - monitor capture with timeout
  - startup pattern validation (`grep -E`)
- Added log artifact upload (`actions/upload-artifact@v4`) with `if: always()`.

### Notes
- This gate remains manual and non-required until runner reliability is characterized.
- Next operational step is one successful manual HIL run record on the self-hosted runner.

## 2026-03-13 - CI/CD slice 5 validation attempt: runner provisioning gap
### Context
Manual `HIL Smoke` dispatch was attempted from GitHub Actions after merge.

### Result
- Workflow queued successfully.
- Job remained pending with `Requested labels: self-hosted`.
- No self-hosted runner was registered/online, so no HIL execution occurred.

### Notes
- This is an infrastructure readiness blocker, not a workflow-definition failure.
- Next step is to provision and register at least one self-hosted runner, then rerun the same manual smoke workflow.

## 2026-03-13 - Runner provisioning scaffold added (containerized)
### Changes
- Added in-repo runner scaffold under `infra/runner/`:
  - `Dockerfile`
  - `entrypoint.sh`
  - `docker-compose.yml`
  - `.env.example`
  - `README.md`
- Added local secret ignore for runner token file:
  - `infra/runner/.env` in `.gitignore`

### Notes
- This keeps runner provisioning reproducible without coupling to the daily dev container.
- Next step is operational bring-up: register runner token, start compose stack, verify runner online, rerun `HIL Smoke`.

## 2026-03-13 - Runner scaffold follow-up: preload ESP-IDF environment
### Context
Initial HIL run on self-hosted runner failed preflight (`command -v idf.py`) because runner process environment did not include ESP-IDF exported paths.

### Changes
- Updated `infra/runner/entrypoint.sh` to source `/opt/esp/idf/export.sh` before starting runner listener.
- Updated runner README notes to document default `idf.py` availability.

## 2026-03-13 - Runner scaffold follow-up: serial device permission mapping
### Context
HIL smoke run reached flash step but failed with:
- `Invalid value for --port: Path '/dev/ttyACM0' is not readable.`

### Changes
- Updated `infra/runner/docker-compose.yml` to add serial group mapping via `group_add`.
- Added `HIL_SERIAL_GID` to `.env.example`.
- Updated runner README with host command to capture device gid and troubleshooting note.

### Notes
- Runner container needs host serial device gid mapped so `runner` user can access `/dev/ttyACM0`.

## 2026-03-13 - HIL smoke follow-up: startup pattern validation hardening
### Context
After port/flash access was fixed, HIL run failed at startup-pattern validation due log-format/pattern mismatch.

### Changes
- Updated default `startup_pattern` in `.github/workflows/hil-smoke.yml` to include structured runtime logs.
- Added ANSI/control-sequence stripping from monitor output before pattern matching.
- Added monitor log tail output in validation step for faster diagnosis on future failures.

## 2026-03-13 - Runner scaffold follow-up: persistent registration state (proper fix)
### Context
Runner restarts/rebuilds could repeatedly trigger `config.sh --replace` and transient session conflicts because only `_work` was persisted.

### Changes
- Updated runner architecture to persist full runner home state:
  - `infra/runner/docker-compose.yml` now mounts `runner-home` to `/home/runner/actions-runner`.
- Updated `infra/runner/Dockerfile`:
  - keeps a template runner install at `/opt/actions-runner-template`
  - runner home is initialized at runtime from template when volume is fresh
- Updated `infra/runner/entrypoint.sh`:
  - root pre-phase sets ownership for mounted runner home
  - runner bootstrap copies template files only when needed
  - `GH_RUNNER_TOKEN` required only for first registration (when `.runner` is absent)
- Updated runner README with one-time migration/reset guidance.

### Notes
- This eliminates routine re-registration churn and reduces startup session conflicts.

## 2026-03-13 - Runner operational note: GitHub session timeout behavior
### Observation
- After stopping the runner container, GitHub may continue showing the runner as `Online/Idle` for a short period before transitioning to `Offline`.
- Restarting the container during this interval can produce temporary:
  - `A session for this runner already exists`

### Conclusion
- This is normal GitHub runner session timeout behavior, not a runner implementation defect.
- Operational guidance: for clean reconnect logs, wait until GitHub marks runner `Offline` before restart.

## 2026-03-13 - CI/CD slice 6 implemented: policy enforcement updates
### Changes
- Updated `docs/DELIVERY_WORKFLOW.md` with explicit required cloud check names:
  - `App Build (ESP-IDF)`
  - `Unit Test App Build (ESP-IDF)`
- Added release-time gate policy:
  - `HIL Smoke` must pass manually on target release commit before creating `v*` tags.
- Updated `README.md` with CI/release/HIL workflow badges and release policy note.

### Operator actions (GitHub settings)
- Apply branch protection rules in GitHub UI:
  - `develop`: require cloud checks + PR reviews
  - `master`: require cloud checks + PR reviews + no direct push
- Keep `HIL Smoke` non-required in branch protection until stability is sufficient for required-gate promotion.

## 2026-03-13 - HIL smoke follow-up: serial diagnostics and flash fallback
### Context
HIL runs intermittently failed at flash with `/dev/ttyACM0` unreadable, while ad-hoc container checks appeared valid.

### Changes
- Added explicit serial diagnostics step in `.github/workflows/hil-smoke.yml`:
  - `whoami`, `id`, `groups`
  - `ls -l` and `stat` on selected serial device
  - readability check output
- Added flash log capture (`hil-logs/flash.log`) for post-failure analysis.
- Reworked flash fallback to avoid running `idf.py` as root:
  - if serial device is unreadable, use `sudo` only to repair port permissions (`chmod a+rw`)
  - then execute `idf.py flash` as the normal runner user.
- Updated runner image user setup to add `runner` to `dialout` group explicitly.

## 2026-03-13 - HIL smoke follow-up: non-interactive monitor TTY workaround
### Context
HIL run reached monitor phase but failed with:
- `Monitor requires standard input to be attached to TTY.`

### Changes
- Updated monitor capture in `.github/workflows/hil-smoke.yml` to run through `script` (pty wrapper):
  - `script -q -c \"idf.py ... monitor\" /dev/null`
- Kept timeout + cleaned-log pipeline unchanged.

## 2026-03-14 - CLI menu command semantics corrected
### Context
CLI v1 originally mirrored physical button semantics by mapping named commands onto existing short/long press events. That was sufficient for initial bring-up, but it exposed an operational mismatch once the CLI surface included explicit menu commands such as `menu enter` and `menu exit`.

### Finding
- `menu enter` and `menu exit` both mapped to the same long-press app event.
- Adapter-side dispatch policy only gated `run`, `pause`, and `run pause toggle`.
- As a result, `menu exit` was still accepted while `running` or `paused`, then interpreted by runtime policy as a long press, which entered the menu instead of exiting it.
- Root cause: the CLI surface had grown beyond a pure "virtual button" model, but command validity rules had not been updated to match the more explicit command vocabulary.

### Changes
- Added a temporary shared CLI state-gating helper:
  - `app_cli_command_map_is_allowed_in_state(...)`
- Centralized command validity rules in core app-plumbing while CLI v1 still mapped commands directly onto short/long press app events.
- Updated the IDF CLI adapter to use the shared helper instead of partial local checks.
- Added command-map tests covering runtime-state gating for menu commands.

### Why
- This preserves a single runtime/menu state machine as the behavioral source of truth while making the CLI adapter semantics match the named commands users actually type.
- The fix is intentionally a state-conditioned adapter rule, not a new CLI state machine.
- A future cleanup may introduce explicit semantic app events for `menu enter` / `menu exit` / `run` / `pause`, but that is not required to make CLI v1 correct and predictable.

### Superseded by later Slice 4A work
- The state-gating helper above was an intermediate corrective step.
- Slice 4A is now refactoring this path so CLI routes explicit blinky command intents and the LED domain decides whether those commands apply in the current runtime state.

### Verification
- `idf.py -D CCACHE_ENABLE=1 build` pass
- unit-test-app targeted build for `core_blinky` + `blinky_idf` pass
- on-device app validation pass
- on-device unit-test-app validation pass
- Unity result: `100 / 0 / 0`

### Follow-up note
- Future CI/CD improvement should land in `.github/workflows/hil-smoke.yml`, not runner container plumbing:
  - add `tools/hil/cli_smoke.py`
  - add automated Unity execution/report capture on hardware
  - upload richer CLI/Unity artifacts on failure

### Commit(s)
- `b9ec755`, `9e36dc8`

## 2026-03-14 - Runtime pause freeze root cause documented
### Changes
- Recorded the root cause of the sine-mode button pause freeze in `led_runtime_step(...)`.
- Captured that the runtime could publish a brightness update earlier in the same step, then transition to `PAUSED` and also request LED-off level output.
- Noted the fix: clear any pending output before applying the state-transition side effects so pause/menu transitions cannot leak stale brightness writes.
- Added a regression test for the pause path:
  - `runtime pause suppresses same-step brightness update`

### Why
- The freeze symptom was not a button debouncing failure; it was an output-ordering bug during runtime state transitions.
- Sine mode made the issue easier to hit because it frequently emits brightness writes, so a short press could collide with an in-flight brightness update on the same step.
- Documenting the exact cause makes it easier to recognize this class of bug later: state transitions must invalidate previously computed output from the old state.

## 2026-03-27 - v0.2.0 docs and release closeout checklist
### Purpose
Keep the current docs/release closeout work near the bottom of the journal so the active branch state is easy to re-enter while preserving the older Slice 4 feature-planning material in place.

- [x] Slice 5: docs and release prep for `v0.2.0`
  - Goal: close the loop on user-facing docs and release framing once config-command scope is stable.
  - Includes: README/devlog/architecture alignment for the CLI+persistence surface and release-prep cleanup.
  - Commit(s): `d34863b`, `231b2d9`, `cefd81a`, `b12ede9`, `7c7fd92`
  - Sub-slices:
    - [x] Slice 5a: map current docs and release-facing surfaces
      - Current docs/release surface map:
        - `README.md` was still a compact build/test entrypoint, but it did not yet explain the CLI control/config surface, the persistence behavior, or where config-command usage belonged in the operator story.
        - `docs/ARCHITECTURE.md` reflected the newer command-routing and persistence boundary decisions, but it still carried older “next slice” planning sections that were no longer the active near-term framing after Slice 4 completion.
        - `docs/PERSISTENCE_SCHEMA.md` captured the stored settings layout and reset/reseed semantics, but the top-level docs did not yet consistently point readers to it when discussing config persistence.
        - `docs/RELEASE_CHECKLIST.md` was structurally aligned with the release workflow, but it had not yet been reviewed against the current `v0.2.0` slice framing or the just-landed CLI/persistence validation surface.
        - `docs/DEVLOG.md` recorded Slice 4 completion and validation, but Slice 5 itself still needed an explicit docs/release cleanup structure before public-facing docs were changed.
      - Notes: the main Slice 5 need appeared to be alignment and framing rather than large missing documents; the current docs mostly existed, but they did not yet present the CLI+persistence feature line as a clean, coherent outside-reader surface.
      - Commit(s): `d34863b`
    - [x] Slice 5b: define the target docs and release shape for `v0.2.0`
      - Target docs/release shape for `v0.2.0`:
        - `README.md` should become the outside-reader/operator entrypoint for the now-stable CLI+persistence surface: what the app does, how to build/flash/test it, the basic runtime-control commands, the basic config-command family, and where deeper docs live.
        - `docs/ARCHITECTURE.md` should stay focused on stable structural and ownership boundaries: runtime flow, command-routing split, persistence boundary, config ownership, and module responsibilities. It should not keep carrying stale “next slice” planning text once those slices are complete.
        - `docs/PERSISTENCE_SCHEMA.md` should remain the canonical home for persisted-settings schema, key layout, seed/reset/reseed semantics, and schema-version notes rather than duplicating that detail in the README.
        - `docs/RELEASE_CHECKLIST.md` should stay a lean operator-facing release runbook: branch/tag flow, RC/final tag steps, and required validation gates. It should reflect the current release process without re-explaining implementation history or broader workflow rationale.
        - `docs/DEVLOG.md` should remain the project's time-series development journal for slice history, rationale, validation notes, and follow-through decisions across the life of the project, rather than trying to serve as the quick-start or public feature summary.
      - Notes: the target shape should make the CLI+persistence feature line easy to understand from the README, easy to reason about structurally from the architecture/schema docs, and easy to release operationally from the release checklist without mixing those roles together again.
      - Commit(s): `231b2d9`
    - [x] Slice 5c: apply one focused docs/release alignment pass
      - Notes: performed a deliberate branch-closeout docs/release coherence pass so the public-facing and stable docs now match the landed CLI control-plane and persistence surface targeted for `v0.2.0`. This pass strengthened the README as the project front door, added succinct “what this is / how to use it / why it matters” framing, clarified the default hardware and button/CLI interaction model, updated architecture wording to match the current persistence implementation and latest on-device test result, and rechecked the release-facing docs for coherence with current branch reality without reopening settled Slice 4 behavior or broadening into a full project documentation rewrite.
      - Commit(s): `cefd81a`
    - [x] Slice 5d: normalize the devlog back toward template shape
      - Notes: moved the active Slice 5 checklist into this newest dated entry and replaced the older in-the-middle copy with a pointer so the current work once again lives near the bottom of the journal without erasing the older Slice 4 planning memo that belongs to the `2026-03-13` feature-kickoff entry.
      - Commit(s): `b12ede9`
    - [x] Slice 5e: document deferred docs/release follow-up
      - Notes: completed the deferred-log follow-up for the docs/release closeout pass by truthfully narrowing older broad items where this branch and later landed work materially advanced them, while preserving the still-open narrower successors for HIL gate maturation, development-environment decoupling, and assert-contract/async-backpressure hardening. This keeps the branch-closeout pass focused on truthful record cleanup rather than forcing unrelated follow-on work into the first `v0.2.0` release branch.
      - Commit(s): `7c7fd92`

## 2026-03-29 - RC release-version hotfix
### Purpose
Capture the release-path hotfix needed after `v0.2.0-rc.1` showed that the built firmware version string was not being derived deterministically from the release tag in CI.

- [ ] Slice 1: stabilize release version tagging in CI
  - Goal: make RC/final release builds report the pushed tag as the app version instead of relying on unstable auto-derived git state.
  - Includes: release workflow version wiring only; do not broaden into general CI redesign during the `v0.2.0` release path.
  - Commit(s):
  - Sub-slices:
    - [ ] Slice 1a: make release builds set explicit project version from the pushed tag
      - Notes: `v0.2.0-rc.1` booted as `v0.2.0-rc.1-dirty`, which is close but still not acceptable for release identity. The intended hotfix is to have the release workflow write `version.txt` from `${GITHUB_REF_NAME}` before the build so RC and final artifacts carry deterministic app version strings.
      - Deferred follow-up: logged `D-2026-03-29-01` to clarify in the workflow/docs that slice numbering is branch-local and new branches restart at `Slice 1`; that rule gap was noticed during this hotfix branch practice but is out of scope for the release-version fix itself.
      - Commit(s):
