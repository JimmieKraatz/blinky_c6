#include "led_startup_policy.h"

static bool led_wave_is_valid(led_wave_t wave)
{
    return (wave >= LED_WAVE_SQUARE && wave < LED_WAVE_COUNT);
}

led_wave_t led_startup_policy_select_wave(const led_startup_config_t *cfg)
{
    if (!cfg || !led_wave_is_valid(cfg->start_wave)) {
        return LED_WAVE_SINE;
    }
    return cfg->start_wave;
}
