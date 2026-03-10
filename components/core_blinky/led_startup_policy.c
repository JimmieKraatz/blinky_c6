#include "led_startup_policy.h"

led_wave_t led_startup_policy_select_wave(const led_startup_config_t *cfg)
{
    if (!cfg) {
        return LED_WAVE_SINE;
    }

    switch (cfg->selector) {
    case LED_STARTUP_SELECT_SQUARE:
        return LED_WAVE_SQUARE;
    case LED_STARTUP_SELECT_SAW_UP:
        return LED_WAVE_SAW_UP;
    case LED_STARTUP_SELECT_SAW_DOWN:
        return LED_WAVE_SAW_DOWN;
    case LED_STARTUP_SELECT_TRIANGLE:
        return LED_WAVE_TRIANGLE;
    case LED_STARTUP_SELECT_SINE:
    case LED_STARTUP_SELECT_DEFAULT:
    default:
        return LED_WAVE_SINE;
    }
}
