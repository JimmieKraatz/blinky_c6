#pragma once

#include "blinky_control_command.h"
#include "led_runtime.h"

typedef enum {
    LED_COMMAND_RESULT_INVALID = 0,
    LED_COMMAND_RESULT_IGNORED,
    LED_COMMAND_RESULT_APPLIED,
} led_command_result_t;

/* Interpret one semantic blinky control command against current LED runtime state. */
led_command_result_t led_command_dispatch(led_runtime_t *rt,
                                          blinky_control_command_t cmd,
                                          blinky_time_ms_t now,
                                          led_runtime_output_t *out);
