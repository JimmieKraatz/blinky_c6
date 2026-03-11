#pragma once

#include <stdbool.h>

#include "blinky_events.h"
#include "blinky_log.h"
#include "blinky_time.h"
#include "led_model.h"
#include "led_policy.h"

typedef struct {
    led_policy_state_t state;
    led_policy_ctx_t policy;
    led_model_t model;
    blinky_log_sink_t *log_sink;
} led_runtime_t;

typedef struct {
    bool write_level;
    bool level_on;
    bool write_brightness;
    led_brightness_t brightness;
} led_runtime_output_t;

/* Contract: rt, cfg, and out are required (non-null). */
void led_runtime_init(led_runtime_t *rt,
                      const led_model_config_t *cfg,
                      led_wave_t start_wave,
                      blinky_time_ms_t now,
                      led_runtime_output_t *out);

/* Contract: rt and out are required (non-null). */
void led_runtime_step(led_runtime_t *rt,
                      blinky_time_ms_t now,
                      blinky_event_t event,
                      led_runtime_output_t *out);

/* Contract: rt is required (non-null). sink may be null to disable logging. */
void led_runtime_set_log_sink(led_runtime_t *rt, blinky_log_sink_t *sink);
