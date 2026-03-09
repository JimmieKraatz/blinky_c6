#pragma once

#include <stdbool.h>

#include "blinky_events.h"
#include "led_model.h"

typedef enum {
    LED_POLICY_RUNNING = 0,
    LED_POLICY_PAUSED,
    LED_POLICY_MENU,
    LED_POLICY_COUNT,
} led_policy_state_t;

typedef struct {
    led_policy_state_t menu_return_state;
    led_wave_t menu_wave;
} led_policy_ctx_t;

typedef struct {
    led_policy_state_t next_state;
    bool menu_wave_changed;
} led_policy_step_result_t;

led_policy_step_result_t led_policy_step(led_policy_ctx_t *ctx,
                                         led_policy_state_t current_state,
                                         led_wave_t current_wave,
                                         blinky_event_t event);

const char *led_policy_wave_name(led_wave_t wave);
