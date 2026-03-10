#pragma once

#include "led_model.h"

typedef enum {
    LED_STARTUP_SELECT_DEFAULT = 0,
    LED_STARTUP_SELECT_SQUARE,
    LED_STARTUP_SELECT_SAW_UP,
    LED_STARTUP_SELECT_SAW_DOWN,
    LED_STARTUP_SELECT_TRIANGLE,
    LED_STARTUP_SELECT_SINE,
} led_startup_selector_t;

typedef struct {
    led_startup_selector_t selector;
} led_startup_config_t;

/* Select initial runtime wave from core-facing startup config.
 * Falls back to SINE when config is null or wave is invalid.
 */
led_wave_t led_startup_policy_select_wave(const led_startup_config_t *cfg);
