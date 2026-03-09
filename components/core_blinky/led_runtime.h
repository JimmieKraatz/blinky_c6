#pragma once

#include <stdbool.h>

#include "blinky_events.h"
#include "blinky_time.h"
#include "led_model.h"
#include "led_policy.h"

typedef struct {
    led_policy_state_t state;
    led_policy_ctx_t policy;
    led_model_t model;
} led_runtime_t;

typedef struct {
    bool write_level;
    bool level_on;
    bool write_brightness;
    led_brightness_t brightness;
} led_runtime_output_t;

void led_runtime_init(led_runtime_t *rt,
                      const led_model_config_t *cfg,
                      led_wave_t start_wave,
                      blinky_time_ms_t now,
                      led_runtime_output_t *out);

void led_runtime_step(led_runtime_t *rt,
                      blinky_time_ms_t now,
                      blinky_event_t event,
                      led_runtime_output_t *out);
