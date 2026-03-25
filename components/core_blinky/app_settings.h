#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "blinky_log.h"
#include "led_startup_policy.h"

typedef struct {
    uint32_t schema_version;
    bool boot_pattern_enabled;
    bool log_intensity_enabled;
    blinky_log_level_t log_min_level;
    led_startup_selector_t startup_selector;
    uint32_t test_counter;
    bool test_mode_enabled;
} app_settings_t;

typedef struct {
    bool boot_pattern_enabled;
    bool log_intensity_enabled;
    blinky_log_level_t log_min_level;
    led_startup_selector_t startup_selector;
} app_settings_defaults_t;

#define APP_SETTINGS_SCHEMA_VERSION 2U

void app_settings_defaults(app_settings_t *cfg, const app_settings_defaults_t *defaults);
bool app_settings_is_valid(const app_settings_t *cfg);
