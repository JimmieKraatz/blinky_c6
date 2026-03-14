#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Placeholder persisted settings payload for Slice 4B contract bring-up.
 * These fields are intentionally simple so we can validate storage plumbing
 * before migrating real config items into NVS.
 */
typedef struct {
    uint32_t schema_version;
    uint32_t test_counter;
    bool test_mode_enabled;
} app_settings_t;

#define APP_SETTINGS_SCHEMA_VERSION 1U

void app_settings_defaults(app_settings_t *cfg);
bool app_settings_is_valid(const app_settings_t *cfg);
