# Persistence Schema

## Overview
This document tracks the persisted settings layout stored in the app-owned NVS partition:

- partition: `appcfg`
- namespace: `blinky`
- storage mode: unencrypted

Persisted settings act as overrides on top of build-time defaults sourced through `_idf` config mapping.
On reset or missing/invalid persisted data, the app reseeds the store from those current defaults.

## Version 2
Current schema version: `2`

Stored keys:
- `schema_ver` (`u32`)
- `boot_pattern` (`u8`, boolean)
- `log_intensity` (`u8`, boolean)
- `log_level` (`u8`, `blinky_log_level_t`)
- `startup_wave` (`u8`, `led_startup_selector_t`)
- `test_count` (`u32`)
- `test_mode` (`u8`, boolean)

Default source rules:
- `boot_pattern` seeds from `BLINKY_BOOT_PATTERN`
- `log_intensity` seeds from `BLINKY_LOG_INTENSITY`
- `log_level` seeds from `BLINKY_LOG_MIN_LEVEL_*`
- `startup_wave` seeds from the current core startup selector default

Load behavior:
- if persisted data matches the current schema and validates, apply overrides
- if persisted data is missing, seed from defaults
- if persisted data is invalid or schema-mismatched, erase persisted data and reseed from defaults

Reset behavior:
- `reset` clears persisted overrides in `appcfg`
- next boot or next settings seed falls back to current build/default config sources

## Notes
- `test_count` and `test_mode` remain scaffold fields from persistence bring-up
- `startup_mode` is not yet modeled in persisted settings
- `boot_pattern_ms` remains Kconfig-backed in the current slice
