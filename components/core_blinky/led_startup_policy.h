#pragma once

#include "led_model.h"

typedef struct {
    led_wave_t start_wave;
} led_startup_config_t;

/* Select initial runtime wave from core-facing startup config.
 * Falls back to SINE when config is null or wave is invalid.
 */
led_wave_t led_startup_policy_select_wave(const led_startup_config_t *cfg);
