#include "led_model.h"

#include <math.h>

/* Per-wave timing and step controls. */
#define LED_MODEL_SAW_STEP_PCT        CONFIG_BLINKY_SAW_STEP_PCT
#define LED_MODEL_PERIOD_MS           CONFIG_BLINKY_WAVE_PERIOD_MS
#define LED_MODEL_SINE_STEPS_RAW      (CONFIG_BLINKY_WAVE_PERIOD_MS / CONFIG_BLINKY_POLL_MS)
#define LED_MODEL_SINE_STEPS_MIN      8U
#define LED_MODEL_SINE_STEPS_CLAMPED  ((LED_MODEL_SINE_STEPS_RAW < LED_MODEL_SINE_STEPS_MIN) ? LED_MODEL_SINE_STEPS_MIN : LED_MODEL_SINE_STEPS_RAW)
#define LED_MODEL_SINE_STEPS          ((LED_MODEL_SINE_STEPS_CLAMPED > CONFIG_BLINKY_SINE_STEPS_MAX) ? CONFIG_BLINKY_SINE_STEPS_MAX : LED_MODEL_SINE_STEPS_CLAMPED)

static uint8_t SINE_PCT_LUT[CONFIG_BLINKY_SINE_STEPS_MAX];
static bool sine_lut_ready = false;

static uint32_t step_ms_for_steps(uint32_t period_ms, uint32_t steps)
{
    if (steps == 0U) {
        return 1U;
    }
    uint32_t ms = (period_ms + (steps / 2U)) / steps;
    return (ms == 0U) ? 1U : ms;
}

static TickType_t ticks_for_steps(uint32_t period_ms, uint32_t steps)
{
    return pdMS_TO_TICKS(step_ms_for_steps(period_ms, steps));
}

static void led_model_build_sine_lut(void)
{
    const float two_pi = 6.28318530717958647692f;
    for (uint32_t i = 0; i < LED_MODEL_SINE_STEPS; ++i) {
        float phase = (two_pi * (float)i) / (float)LED_MODEL_SINE_STEPS;
        /* Scale sine from [-1,1] to [0,100] percent. */
        float pct = (sinf(phase) + 1.0f) * 50.0f;
        if (pct < 0.0f) {
            pct = 0.0f;
        } else if (pct > 100.0f) {
            pct = 100.0f;
        }
        SINE_PCT_LUT[i] = (uint8_t)(pct + 0.5f);
    }
    sine_lut_ready = true;
}

void led_model_init(led_model_t *model)
{
    if (!sine_lut_ready) {
        led_model_build_sine_lut();
    }
    model->phase_idx = 0;
}

uint32_t led_model_sine_steps(void)
{
    return (uint32_t)LED_MODEL_SINE_STEPS;
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

void led_model_set_wave(led_model_t *model,
                        led_wave_t wave,
                        TickType_t now)
{
    if (!sine_lut_ready) {
        led_model_build_sine_lut();
    }
    model->wave = wave;

    switch (wave) {
        case LED_WAVE_SAW_UP:
            model->pct = 0;
            model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, 100U / LED_MODEL_SAW_STEP_PCT);
            break;
        case LED_WAVE_SAW_DOWN:
            model->pct = 100;
            model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, 100U / LED_MODEL_SAW_STEP_PCT);
            break;
        case LED_WAVE_TRIANGLE:
            model->pct = 0;
            model->rising = true;
            model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, (100U / LED_MODEL_SAW_STEP_PCT) * 2U);
            break;
        case LED_WAVE_SINE:
            model->phase_idx = 0;
            model->pct = SINE_PCT_LUT[model->phase_idx];
            model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, LED_MODEL_SINE_STEPS);
            break;
        case LED_WAVE_SQUARE:
        default:
            model->level = false;
            model->pct = 0;
            model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, 2U);
            break;
    }
}

bool led_model_tick(led_model_t *model,
                    TickType_t now,
                    led_percent_t *pct_out)
{
    switch (model->wave) {
        case LED_WAVE_SAW_UP:
            if (now >= model->next_update) {
                led_percent_t next = (led_percent_t)(model->pct + LED_MODEL_SAW_STEP_PCT);
                if (next > 100) {
                    next = 0;
                }
                model->pct = next;
                model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, 100U / LED_MODEL_SAW_STEP_PCT);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_SAW_DOWN:
            if (now >= model->next_update) {
                led_percent_t next = (model->pct < LED_MODEL_SAW_STEP_PCT)
                                   ? 100
                                   : (uint8_t)(model->pct - LED_MODEL_SAW_STEP_PCT);
                model->pct = next;
                model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, 100U / LED_MODEL_SAW_STEP_PCT);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_TRIANGLE:
            if (now >= model->next_update) {
                if (model->rising) {
                    led_percent_t next = (led_percent_t)(model->pct + LED_MODEL_SAW_STEP_PCT);
                    if (next >= 100) {
                        model->pct = 100;
                        model->rising = false;
                    } else {
                        model->pct = next;
                    }
                } else {
                    if (model->pct <= LED_MODEL_SAW_STEP_PCT) {
                        model->pct = 0;
                        model->rising = true;
                    } else {
                        model->pct = (uint8_t)(model->pct - LED_MODEL_SAW_STEP_PCT);
                    }
                }

                model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, (100U / LED_MODEL_SAW_STEP_PCT) * 2U);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_SINE:
            if (now >= model->next_update) {
                model->phase_idx = (uint8_t)((model->phase_idx + 1U) % LED_MODEL_SINE_STEPS);
                model->pct = SINE_PCT_LUT[model->phase_idx];
                model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, LED_MODEL_SINE_STEPS);
                *pct_out = model->pct;
                return true;
            }
            break;
        case LED_WAVE_SQUARE:
        default:
            if (now >= model->next_update) {
                model->level = !model->level;
                model->pct = model->level ? 100 : 0;
                model->next_update = now + ticks_for_steps(LED_MODEL_PERIOD_MS, 2U);
                *pct_out = model->pct;
                return true;
            }
            break;
    }

    return false;
}

bool led_model_tick_raw(led_model_t *model,
                        TickType_t now,
                        led_brightness_t *brightness_out)
{
    led_percent_t pct = 0;
    if (!led_model_tick(model, now, &pct)) {
        return false;
    }

    *brightness_out = led_brightness_from_percent(pct);
    return true;
}
