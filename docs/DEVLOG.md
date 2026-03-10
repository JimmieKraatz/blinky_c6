# Development Log

## Why this exists
This file is a diary of development progress: what changed, why, and what is next.
For the stable technical view, see `docs/ARCHITECTURE.md`.

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
Completed the prepatory boundary work for LED and button to reduce coupling before event/HSM migration.

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
