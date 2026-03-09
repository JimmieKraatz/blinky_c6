#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "button_logic.h"

typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS
} button_event_t;

typedef enum {
    BUTTON_PULL_NONE = 0,
    BUTTON_PULL_UP,
    BUTTON_PULL_DOWN
} button_pull_t;

typedef struct {
    gpio_num_t gpio;
    bool active_low;
    button_logic_t logic;
} button_t;

/* Configure GPIO for a debounced button input. */
void button_init(button_t *btn,
                 gpio_num_t gpio,
                 bool active_low,
                 button_pull_t pull,
                 button_logic_debounce_t debounce_count,
                 blinky_time_ms_t long_press_ms);

/* Return button events based on debounced edges and hold duration. */
button_event_t button_poll_event(button_t *btn, TickType_t now);

/* Test helper: drive the debouncer with a raw pressed value. */
button_event_t button_poll_event_raw(button_t *btn, bool raw_pressed, TickType_t now);
