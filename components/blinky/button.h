#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS
} button_event_t;

typedef struct {
    gpio_num_t gpio;
    bool active_low;
    bool stable;
    uint8_t debounce;
    uint8_t debounce_count;
    TickType_t pressed_at;
    TickType_t long_press_ticks;
    bool long_reported;
} button_t;

/* Configure GPIO for a debounced button input. */
void button_init(button_t *btn,
                 gpio_num_t gpio,
                 bool active_low,
                 uint8_t debounce_count,
                 uint32_t long_press_ms);

/* Return button events based on debounced edges and hold duration. */
button_event_t button_poll_event(button_t *btn, TickType_t now);

/* Test helper: drive the debouncer with a raw pressed value. */
button_event_t button_poll_event_raw(button_t *btn, bool raw_pressed, TickType_t now);
