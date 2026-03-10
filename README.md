# blinky_c6

ESP-IDF app for ESP32-C6 that drives an LED using a waveform-based state machine. A single button controls run/pause and a wave-selection menu.

For deeper docs:
- Architecture and module boundaries: `docs/ARCHITECTURE.md`
- Refactor journey and decisions: `docs/DEVLOG.md`

## Quick Start

### Requirements
- ESP-IDF installed and activated (e.g. `export.sh` on Linux/macOS)
- Target configured for ESP32-C6

### Configure Target
```bash
idf.py set-target esp32c6
```

### Build
```bash
idf.py -D CCACHE_ENABLE=1 build
```

### Flash and Monitor
```bash
idf.py -p /dev/ttyUSB0 flash monitor
```
Replace the port with your device (`/dev/ttyACM0` on Linux, `COMx` on Windows).

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

## License
MIT. See `LICENSE`.
