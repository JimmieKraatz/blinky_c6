#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "fsm_engine.h"
#include "button.h"
#include "led_model.h"

/* LED state machine context.
 * Owns runtime state for LED behavior and local button debounce.
 */
typedef struct {
    fsm_state_t state;
    button_t button;
    uint8_t menu_return_state;
    led_wave_t menu_wave;

    led_model_t model;
} sm_led_ctx_t;

/* Initialize LED hardware and enter initial LED FSM state. */
void led_sm_init(sm_led_ctx_t *ctx);
/* Delay/poll once and execute one FSM step. */
void led_sm_step(sm_led_ctx_t *ctx);
