#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "fsm_engine.h"
#include "button_input_adapter.h"
#include "button_input_adapter_idf.h"
#include "led_model.h"
#include "led_policy.h"
#include "led_output_adapter.h"
#include "led_output_adapter_idf.h"

/* LED state machine context.
 * Owns runtime state for LED behavior and local button debounce.
 */
typedef struct {
    fsm_state_t state;
    button_input_adapter_t input;
    button_input_adapter_idf_t input_idf;
    led_policy_ctx_t policy;

    led_output_adapter_t led_output;
    led_output_adapter_idf_t led_output_idf;
    led_model_t model;
} sm_led_ctx_t;

/* Initialize LED hardware and enter initial LED FSM state. */
void led_sm_init(sm_led_ctx_t *ctx);
/* Delay/poll once and execute one FSM step. */
void led_sm_step(sm_led_ctx_t *ctx);
