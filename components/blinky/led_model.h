#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

/* Time-driven LED waveform generator.
 * This module is hardware-agnostic: callers can consume either percent or
 * normalized raw brightness values.
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

typedef struct {
    led_wave_t wave;
    TickType_t next_update;
    bool level;
    bool rising;
    uint8_t phase_idx;
    uint8_t pct;
} led_model_t;

/* Initialize model state (precompute sine LUT if needed). */
void led_model_init(led_model_t *model);

/* Conversion helpers between UI percent and normalized raw brightness. */
led_brightness_t led_brightness_from_percent(uint8_t pct);
uint8_t led_percent_from_brightness(led_brightness_t brightness);

/* Number of sine steps derived from config (build-time constant). */
uint32_t led_model_sine_steps(void);

/* Select waveform and reset model phase/output. */
void led_model_set_wave(led_model_t *model,
                        led_wave_t wave,
                        TickType_t now);

/* Advance model state in percent. Returns true when a new value is produced. */
bool led_model_tick(led_model_t *model,
                    TickType_t now,
                    uint8_t *pct_out);

/* Same model step, but returns normalized raw brightness. */
bool led_model_tick_raw(led_model_t *model,
                        TickType_t now,
                        led_brightness_t *brightness_out);
