#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "blinky_time.h"

typedef enum {
    BUTTON_LOGIC_EVENT_NONE = 0,
    BUTTON_LOGIC_EVENT_SHORT_PRESS,
    BUTTON_LOGIC_EVENT_LONG_PRESS
} button_logic_event_t;

typedef uint8_t button_logic_debounce_t;

typedef struct {
    bool stable;
    button_logic_debounce_t debounce;
    button_logic_debounce_t debounce_count;
    blinky_time_ms_t pressed_at_ms;
    blinky_time_ms_t long_press_ms;
    bool long_reported;
} button_logic_t;

/* Contract: logic is required (non-null). */
void button_logic_init(button_logic_t *logic,
                       button_logic_debounce_t debounce_count,
                       blinky_time_ms_t long_press_ms);

/* Contract: logic is required (non-null). */
button_logic_event_t button_logic_poll_raw(button_logic_t *logic,
                                           bool raw_pressed,
                                           blinky_time_ms_t now_ms);
