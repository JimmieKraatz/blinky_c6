# Development Log

## Why this exists
This file is a diary of development progress: what changed, why, and what is next.
For the stable technical view, see `docs/ARCHITECTURE.md`.

## Logging Conventions
- Slice naming: use numeric IDs only (`Slice 1`, `Slice 2`, ...). Avoid lettered slices for new entries.
- Module naming: prefer `app_` for app-plumbing contracts and `blinky_` for domain behavior; treat older `led_*` orchestrator names as legacy until refactored.

## 2026-03-13 - CLI naming alignment pass
### Context
As CLI control-plane scaffolding was added, naming drift appeared between older `led_*` mapping/factory terms and app-layer event plumbing.

### Changes
- Renamed `led_event_factory.*` -> `app_event_factory.*`.
- Renamed `led_cli_command_map.*` -> `app_cli_command_map.*`.
- Updated references and tests to use new app-layer naming consistently.

### Notes
- `blinky_cli_command_t` remains intentionally domain-scoped (`blinky_`), while mapping/factory modules stay app-plumbing scoped (`app_`).

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

### Planned slices
1. Slice 1: CLI contracts + command/event mapping model (core + interfaces)
2. Slice 2: IDF CLI adapter (stdin/uart line reader + dispatch bridge)
3. Slice 3: command set v1 parity with button/menu actions:
   - `run`, `pause`
   - `menu enter`, `menu next`, `menu exit`
   - `status`, `help`
4. Slice 4: persistence hooks (NVS-backed config save/load/reset) and tests
5. Slice 5: docs/release prep for `v0.2.0`

### Slice status
- Slice 1 completed (2026-03-13):
  - added CLI command contract in `blinky_interfaces` (`blinky_cli_command_t`)
  - added core mapping module `app_cli_command_map.*`
  - extended event factory with CLI path (`app_event_factory_from_cli_command(...)`)
  - added/updated unit tests for command mapping and event factory behavior
  - completed naming alignment pass (`led_*` -> `app_*` for app-layer plumbing)
- Slice 2 completed (2026-03-13):
  - added IDF UART CLI adapter (`app_cli_adapter_idf.*`) with non-blocking line read on UART0
  - added command parsing bridge from text input to `blinky_cli_command_t`
  - wired CLI command dispatch into existing app-event queue via `app_event_factory_from_cli_command(...)`
  - integrated adapter init/step into `led_sm_init(...)` / `led_sm_step(...)`
  - added Kconfig controls:
    - `BLINKY_CLI_ENABLE`
    - `BLINKY_CLI_UART_RX_BUF_SIZE`
- Slice 3 completed (2026-03-13 to 2026-03-14):
  - added dedicated parser module (`app_cli_parse.*`) for CLI command vocabulary
  - moved parser ownership to `core_blinky` (platform-agnostic placement)
  - implemented command-set parity for:
    - `run`, `pause`, `menu enter`, `menu next`, `menu exit`, `status`, `help`
  - extended command intent model with explicit `BLINKY_CLI_CMD_RUN` / `BLINKY_CLI_CMD_PAUSE`
  - added state-aware dispatch policy so `run`/`pause` are idempotent and ignored outside running/paused states
  - added parser unit tests and command-map coverage updates
  - verification: `idf.py -D CCACHE_ENABLE=1 build` pass
  - added repeatable on-target helper: `tools/hil/cli_smoke.py`
  - fixed console transport mismatch by adding USB Serial/JTAG input backend when `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y` (UART fallback preserved)
  - added interactive console echo/edit behavior in IDF CLI adapter (typed character echo, backspace handling, CR/LF submit compatibility)
  - improved smoke helper serial write/read behavior for deterministic timeout handling in containerized dev environments
  - runtime validation complete: on-target `tools/hil/cli_smoke.py` passed all 7 commands on `/dev/ttyACM0`
  - post-smoke monitor validation complete: `idf.py monitor` showed stable boot to `running` and interactive typed characters were visible over USB Serial/JTAG

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

## Deferred TODOs
- CI/CD rollout with hardware constraints (High Priority):
  - implement cloud GitHub Actions for hardware-independent checks (build, unit-test-app build, lint/format when configured)
  - add self-hosted GitHub Actions runner on devkit host for HIL flash/smoke validation
  - promote HIL to required gate for `develop` -> `master` before release tagging
  - reference policy and acceptance criteria in `docs/DELIVERY_WORKFLOW.md`
- Dev-ability/reproducibility hardening:
  - move or mirror minimum required devcontainer/docker source into this repo
  - ensure a new contributor can build/test without external scaffold dependencies
  - document optional SSH agent forwarding separately from required build steps
- Bootstrap layering split:
  - separate environment/bootstrap config concerns from runtime orchestration
  - revisit `sdkconfig` defaults vs runtime provisioning for future network features
- Dedicated test-hardening branch:
  - strengthen async timing/overflow assertions without expanding refactor branch scope
  - split build/link validation vs on-target assertion validation in workflow/docs
  - require at least one on-device Unity run (`flash monitor` + `*`) before branch merge
  - add explicit assert-contract validation for strict core APIs:
    - verify `led_policy_step(NULL, ...)` triggers assert-fail path
    - keep positive-path coverage proving no assert for valid non-null contexts
- Runtime context ownership follow-up:
  - readdress temporary `app_main` static `sm_led_ctx_t` workaround added to avoid main-task stack overflow
  - align final approach with documented non-singleton lifecycle ownership decision from critical review slice 1
- Fault/shutdown semantics (deferred):
  - define core-owned handling contract before adding platform producers
- Pause behavior policy decision (deferred):
  - decide whether `PAUSED` should freeze LED at current brightness or force LED off
  - document rationale and align tests with final behavior
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
