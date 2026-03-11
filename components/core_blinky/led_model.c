#include "led_model.h"

#include <assert.h>
#include <math.h>

#define LED_MODEL_SINE_STEPS_MIN 8U
#define LED_MODEL_SINE_STEPS_CAP 1024U

static uint32_t clamp_u32(uint32_t value, uint32_t min, uint32_t max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static uint32_t step_ms_for_steps(uint32_t period_ms, uint32_t steps)
{
    if (steps == 0U) {
        return 1U;
    }
    uint32_t ms = (period_ms + (steps / 2U)) / steps;
    return (ms == 0U) ? 1U : ms;
}

static void led_model_build_sine_lut(led_model_t *model)
{
    const float two_pi = 6.28318530717958647692f;
    for (uint32_t i = 0; i < model->sine_steps; ++i) {
        float phase = (two_pi * (float)i) / (float)model->sine_steps;
        float pct = (sinf(phase) + 1.0f) * 50.0f;
        if (pct < 0.0f) {
            pct = 0.0f;
        } else if (pct > 100.0f) {
            pct = 100.0f;
        }
        model->sine_pct_lut[i] = (uint8_t)(pct + 0.5f);
    }
    model->sine_lut_len = model->sine_steps;
}

void led_model_init(led_model_t *model, const led_model_config_t *cfg)
{
    assert(model);
    assert(cfg);

    uint32_t raw_steps = 0;

    model->cfg.wave_period_ms = (cfg->wave_period_ms == 0U) ? 1000U : cfg->wave_period_ms;
    model->cfg.poll_ms = (cfg->poll_ms == 0U) ? 10U : cfg->poll_ms;
    model->cfg.saw_step_pct = (uint8_t)clamp_u32(cfg->saw_step_pct, 1U, 20U);
    model->cfg.sine_steps_max = (uint16_t)clamp_u32(cfg->sine_steps_max,
                                                    LED_MODEL_SINE_STEPS_MIN,
                                                    LED_MODEL_SINE_STEPS_CAP);

    raw_steps = model->cfg.wave_period_ms / model->cfg.poll_ms;
    raw_steps = clamp_u32(raw_steps, LED_MODEL_SINE_STEPS_MIN, model->cfg.sine_steps_max);
    model->sine_steps = (uint16_t)raw_steps;

    model->phase_idx = 0;
    model->sine_lut_len = 0;
    led_model_build_sine_lut(model);
}

uint32_t led_model_sine_steps(const led_model_t *model)
{
    assert(model);
    return model->sine_steps;
}

led_brightness_t led_brightness_from_percent(led_percent_t pct)
{
    if (pct > 100) {
        pct = 100;
    }

    return (led_brightness_t)((((uint32_t)pct * LED_BRIGHTNESS_MAX) + 50U) / 100U);
}

uint8_t led_percent_from_brightness(led_brightness_t brightness)
{
    uint32_t pct = (((uint32_t)brightness * 100U) + (LED_BRIGHTNESS_MAX / 2U)) / LED_BRIGHTNESS_MAX;
    return (uint8_t)pct;
}

void led_model_set_wave(led_model_t *model, led_wave_t wave, blinky_time_ms_t now_ms)
{
    assert(model);
    model->wave = wave;

    switch (wave) {
        case LED_WAVE_SAW_UP:
            model->pct = 0;
            model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                            100U / model->cfg.saw_step_pct);
            break;
        case LED_WAVE_SAW_DOWN:
            model->pct = 100;
            model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                            100U / model->cfg.saw_step_pct);
            break;
        case LED_WAVE_TRIANGLE:
            model->pct = 0;
            model->rising = true;
            model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                            (100U / model->cfg.saw_step_pct) * 2U);
            break;
        case LED_WAVE_SINE:
            model->phase_idx = 0;
            model->pct = model->sine_pct_lut[model->phase_idx];
            model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                            model->sine_steps);
            break;
        case LED_WAVE_SQUARE:
        default:
            model->level = false;
            model->pct = 0;
            model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms, 2U);
            break;
    }
}

bool led_model_tick(led_model_t *model, blinky_time_ms_t now_ms, led_percent_t *pct_out)
{
    assert(model);
    assert(pct_out);

    switch (model->wave) {
        case LED_WAVE_SAW_UP:
            if (now_ms >= model->next_update) {
                led_percent_t next = (led_percent_t)(model->pct + model->cfg.saw_step_pct);
                if (next > 100) {
                    next = 0;
                }
                model->pct = next;
                model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                                100U / model->cfg.saw_step_pct);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_SAW_DOWN:
            if (now_ms >= model->next_update) {
                led_percent_t next = (model->pct < model->cfg.saw_step_pct)
                                   ? 100
                                   : (uint8_t)(model->pct - model->cfg.saw_step_pct);
                model->pct = next;
                model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                                100U / model->cfg.saw_step_pct);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_TRIANGLE:
            if (now_ms >= model->next_update) {
                if (model->rising) {
                    led_percent_t next = (led_percent_t)(model->pct + model->cfg.saw_step_pct);
                    if (next >= 100) {
                        model->pct = 100;
                        model->rising = false;
                    } else {
                        model->pct = next;
                    }
                } else {
                    if (model->pct <= model->cfg.saw_step_pct) {
                        model->pct = 0;
                        model->rising = true;
                    } else {
                        model->pct = (uint8_t)(model->pct - model->cfg.saw_step_pct);
                    }
                }

                model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                                (100U / model->cfg.saw_step_pct) * 2U);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_SINE:
            if (now_ms >= model->next_update) {
                model->phase_idx = (uint16_t)((model->phase_idx + 1U) % model->sine_steps);
                model->pct = model->sine_pct_lut[model->phase_idx];
                model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms,
                                                                model->sine_steps);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_SQUARE:
        default:
            if (now_ms >= model->next_update) {
                model->level = !model->level;
                model->pct = model->level ? 100 : 0;
                model->next_update = now_ms + step_ms_for_steps(model->cfg.wave_period_ms, 2U);
                *pct_out = model->pct;
                return true;
            }
            break;
    }

    return false;
}

bool led_model_tick_raw(led_model_t *model,
                        blinky_time_ms_t now_ms,
                        led_brightness_t *brightness_out)
{
    assert(model);
    assert(brightness_out);

    led_percent_t pct = 0;
    if (!led_model_tick(model, now_ms, &pct)) {
        return false;
    }

    *brightness_out = led_brightness_from_percent(pct);
    return true;
}
