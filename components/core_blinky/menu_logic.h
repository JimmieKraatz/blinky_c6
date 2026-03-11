#pragma once

#include "blinky_events.h"
#include "led_model.h"

typedef enum {
    LED_MENU_ACTION_NONE = 0,
    LED_MENU_ACTION_WAVE_CHANGED,
    LED_MENU_ACTION_EXIT
} led_menu_action_t;

/* Contract: wave is required (non-null). */
led_menu_action_t led_menu_handle_event(led_wave_t *wave, blinky_event_t ev);
