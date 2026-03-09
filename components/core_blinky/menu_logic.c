#include "menu_logic.h"

led_menu_action_t led_menu_handle_event(led_wave_t *wave, blinky_event_t ev)
{
    if (ev == BLINKY_EVENT_SHORT_PRESS) {
        *wave = (led_wave_t)((*wave + 1U) % LED_WAVE_COUNT);
        return LED_MENU_ACTION_WAVE_CHANGED;
    }
    if (ev == BLINKY_EVENT_LONG_PRESS) {
        return LED_MENU_ACTION_EXIT;
    }
    return LED_MENU_ACTION_NONE;
}
