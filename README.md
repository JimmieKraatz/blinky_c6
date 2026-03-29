# blinky_c6

ESP-IDF firmware project for ESP32-C6 implementing a modular embedded control system with runtime CLI interaction and persistent configuration using NVS.

The system demonstrates separation of portable control logic from platform-specific adapters, along with a reproducible development, testing, and release workflow.

It showcases:
- runtime CLI and button-driven control of system behavior
- NVS-backed persistent configuration for startup and logging
- portable core state-machine logic
- explicit adapter boundaries between reusable logic and ESP-IDF integration
- a delivery workflow with tests, CI, releases, and manual HIL validation

## CI/CD Status
[![CI](https://github.com/JimmieKraatz/blinky_c6/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/JimmieKraatz/blinky_c6/actions/workflows/ci.yml)
[![Release](https://github.com/JimmieKraatz/blinky_c6/actions/workflows/release.yml/badge.svg)](https://github.com/JimmieKraatz/blinky_c6/actions/workflows/release.yml)
[![HIL Smoke](https://github.com/JimmieKraatz/blinky_c6/actions/workflows/hil-smoke.yml/badge.svg)](https://github.com/JimmieKraatz/blinky_c6/actions/workflows/hil-smoke.yml)

For deeper docs:
- Architecture and module boundaries: `docs/ARCHITECTURE.md`
- Persistence schema and reset/reseed behavior: `docs/PERSISTENCE_SCHEMA.md`
- Refactor journey and decisions: `docs/DEVLOG.md`
- Delivery workflow (branching, CI/CD, release/tag policy): `docs/DELIVERY_WORKFLOW.md`
- Release operator runbook: `docs/RELEASE_CHECKLIST.md`

Release policy note:
- Cloud checks are required for PR merges.
- `HIL Smoke` remains manual/non-required in branch protection, but is required by release policy before creating `v*` tags.
- Tagged releases publish firmware artifacts through GitHub Releases, including flashable binaries and supporting metadata/checksums.

## What This Repo Demonstrates
- modular embedded design with clear separation between portable policy/runtime logic and ESP-IDF adapters
- reproducible development and validation flow using ESP-IDF builds, Unity unit tests, CI workflows, and release artifacts
- command/control discipline where runtime-control commands and persisted configuration commands use distinct lanes without collapsing architecture boundaries

## How To Use It
Briefly:
- connect an LED to `GPIO_NUM_14`, which is configured as a PWM-driven active-low sink output
- wire the LED so the pin sinks current when the LED is on
- include a series current-limiting resistor sized for your LED and supply voltage; `330 ohm` is a common starting point, and roughly `220-1k ohm` is a typical safe bring-up range
- connect a button to `GPIO_NUM_9` configured as active-low with a pull-up
- flash the app and open a serial monitor

Button behavior:
- short press: pause or resume the waveform
- long press (~3 s): enter or exit the wave-selection menu
- while in the menu, short press cycles the wave type

Terminal commands:
- use `help` to discover runtime and config command families
- use runtime commands such as `status`, `run`, `pause`, and `menu ...` for live control
- use config commands such as `config show`, `config save`, `config reset`, and the startup/logging setters for persisted preferences

Example interaction:
```text
help
status
config show
config startup wave triangle
config log level debug
config save
```

System behavior updates immediately and persists across reboot when saved.

Example config flow:
```text
help config
config show
config startup wave triangle
config log level debug
config save
```

Config changes are applied immediately in-memory and can be persisted to NVS with `config save`. `config reset` clears persisted overrides and returns to default-backed settings.

## Quick Start

### Requirements
- ESP-IDF installed and activated (e.g. `export.sh` on Linux/macOS)

### Build
```bash
idf.py -D CCACHE_ENABLE=1 build
```

### Flash and Monitor
```bash
idf.py -p <your-active-port> flash monitor
```
Replace `<your-active-port>` with the serial device currently assigned to your board (for example `/dev/ttyACM0`, `/dev/ttyUSB0`, or `COMx`).

After the monitor opens, type `help` or `help config` to discover the available command surfaces.

## Unit Tests (Unity via ESP-IDF Unit Test App)
Build the Unit Test App and point it at this repo's components via `EXTRA_COMPONENT_DIRS`.

Option A (terminal):
```bash
idf.py -C $IDF_PATH/tools/unit-test-app -B $PWD/build/unit-test-app -D EXTRA_COMPONENT_DIRS=$PWD/components -D "SDKCONFIG_DEFAULTS=$IDF_PATH/tools/unit-test-app/sdkconfig.defaults;$PWD/test/unit-test-app.sdkconfig.defaults" set-target esp32c6
idf.py -C $IDF_PATH/tools/unit-test-app -B $PWD/build/unit-test-app -D EXTRA_COMPONENT_DIRS=$PWD/components -D SDKCONFIG=$PWD/build/unit-test-app/sdkconfig -D "SDKCONFIG_DEFAULTS=$IDF_PATH/tools/unit-test-app/sdkconfig.defaults;$PWD/test/unit-test-app.sdkconfig.defaults" -D CCACHE_ENABLE=1 -T core_sm -T core_blinky -T blinky_idf -T blinky_interfaces build
idf.py -C $IDF_PATH/tools/unit-test-app -B $PWD/build/unit-test-app -D SDKCONFIG=$PWD/build/unit-test-app/sdkconfig -D "SDKCONFIG_DEFAULTS=$IDF_PATH/tools/unit-test-app/sdkconfig.defaults;$PWD/test/unit-test-app.sdkconfig.defaults" -T core_sm -T core_blinky -T blinky_idf -T blinky_interfaces -p /dev/ttyACM0 flash monitor
```

At the Unity prompt, run all tests with `*` then Enter.

Option B (VS Code Tasks):
- `IDF: Set Target (tests)`
- `IDF: Build (tests)`
- `IDF: Flash+Monitor (tests)`

## Project Layout
- `main/` app entry (`app_main`)
- `components/core_sm/` generic FSM/HSM engine headers + tests
- `components/core_blinky/` generic blinky model/policy/runtime + tests
- `components/blinky_interfaces/` framework-agnostic adapter contracts
- `components/blinky_idf/` ESP-IDF integration/adapters + tests
- `test/` shared test config defaults for unit-test-app

## Notes
- `README.md` provides a high-level overview. Use the linked docs above for detailed architecture, persistence, delivery workflow, and project-history information.

## License
MIT. See `LICENSE`.
