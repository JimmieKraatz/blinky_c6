#pragma once

#include "blinky_time.h"
#include "button_logic.h"

typedef struct {
    button_logic_debounce_t debounce_count;
    blinky_time_ms_t long_press_ms;
} button_policy_timing_t;

/* Normalize timing policy defaults for button logic/runtime use. */
button_policy_timing_t button_policy_timing_normalize(button_policy_timing_t timing);
