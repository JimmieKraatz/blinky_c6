#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "blinky_time.h"

/* Time-driven LED waveform generator.
 * This module is framework/hardware-agnostic.
 */
typedef enum {
    LED_WAVE_SQUARE = 0,
    LED_WAVE_SAW_UP,
    LED_WAVE_SAW_DOWN,
    LED_WAVE_TRIANGLE,
    LED_WAVE_SINE,
    LED_WAVE_COUNT
} led_wave_t;

typedef uint16_t led_brightness_t;
#define LED_BRIGHTNESS_MAX ((led_brightness_t)65535U)
typedef uint8_t led_percent_t;

typedef struct {
    uint32_t wave_period_ms;
    uint32_t poll_ms;
    uint16_t sine_steps_max;
    uint8_t saw_step_pct;
} led_model_config_t;

typedef struct {
    led_wave_t wave;
    blinky_time_ms_t next_update;
    bool level;
    bool rising;
    uint16_t phase_idx;
    uint8_t pct;

    led_model_config_t cfg;
    uint16_t sine_steps;
    uint16_t sine_lut_len;
    uint8_t sine_pct_lut[1024];
} led_model_t;

/* Initialize model state and sanitize runtime configuration.
 * Contract: model and cfg are required (non-null).
 */
void led_model_init(led_model_t *model, const led_model_config_t *cfg);

/* Conversion helpers between UI percent and normalized raw brightness. */
led_brightness_t led_brightness_from_percent(led_percent_t pct);
uint8_t led_percent_from_brightness(led_brightness_t brightness);

/* Number of sine steps derived from runtime config.
 * Contract: model is required (non-null).
 */
uint32_t led_model_sine_steps(const led_model_t *model);

/* Select waveform and reset model phase/output.
 * Contract: model is required (non-null).
 */
void led_model_set_wave(led_model_t *model,
                        led_wave_t wave,
                        blinky_time_ms_t now_ms);

/* Advance model state in percent. Returns true when a new value is produced.
 * Contract: model and pct_out are required (non-null).
 */
bool led_model_tick(led_model_t *model,
                    blinky_time_ms_t now_ms,
                    led_percent_t *pct_out);

/* Same model step, but returns normalized raw brightness.
 * Contract: model and brightness_out are required (non-null).
 */
bool led_model_tick_raw(led_model_t *model,
                        blinky_time_ms_t now_ms,
                        led_brightness_t *brightness_out);
