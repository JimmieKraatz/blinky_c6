# Development Log

## Why this exists
This file captures the implementation journey: decisions, tradeoffs, and refactor rationale.

## Milestones
1. Initial monolithic component (`components/blinky`) with app logic, hardware integration, and tests together.
2. Decoupling refactor into three modules:
   - `core_sm` for reusable state machine engines
   - `core_blinky` for reusable blinky domain logic
   - `blinky_idf` for ESP-IDF/hardware glue
3. Test workflow stabilization:
   - Explicit unit-test-app commands documented
   - VS Code tasks aligned with terminal commands
   - Unit-test `SDKCONFIG_DEFAULTS` layering fixed for reliable Unity runner behavior
   - Serial console defaults tuned for interactive Unity menu input

## Design notes
- Core logic moved out of IDF-dependent code first, then tests were moved with ownership.
- This keeps portability and testability high while retaining a thin IDF integration layer.
- Unity on target is interactive by design (menu-based over serial). CI/HIL automation should drive menu input (`*`, specific case numbers, or tags).

## Known follow-ups
- Optional: add a hardware-in-the-loop CI job that flashes unit-test-app and drives Unity menu input automatically.
- Optional: add diagrams for event/data flow between `blinky_idf` and `core_*` modules.

## Current Plan
Next intended milestone: transition to an event-driven runtime model.

Why:
- It aligns with the introduction of HSM support (`core_sm/hsm_engine.h`).
- It separates event production (hardware/time) from event handling (state orchestration).
- It should make behavior easier to reason about than ad-hoc polling transitions as complexity grows.

Execution intent:
1. Define event types and ownership boundaries.
2. Introduce queue/dispatch in `blinky_idf`.
3. Move orchestration paths onto HSM while preserving behavior covered by current tests.
