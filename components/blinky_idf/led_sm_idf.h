#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_dispatcher.h"
#include "app_event_queue.h"
#include "button_input_adapter.h"
#include "button_input_adapter_idf.h"
#include "led_runtime.h"
#include "led_output_adapter.h"
#include "led_output_adapter_idf.h"

/* LED state machine context.
 * Owns runtime state for LED behavior and local button debounce.
 */
typedef struct {
    button_input_adapter_t input;
    button_input_adapter_idf_t input_idf;

    led_output_adapter_t led_output;
    led_output_adapter_idf_t led_output_idf;
    led_runtime_t runtime;
    app_event_queue_t queue;
    app_dispatcher_t dispatcher;
} sm_led_ctx_t;

/* Initialize LED hardware and enter initial LED FSM state. */
void led_sm_init(sm_led_ctx_t *ctx);

/* Producer side: sample inputs and enqueue one semantic app event. */
void led_sm_producer_step(sm_led_ctx_t *ctx);
/* Consumer side: dispatch queued events (0 => drain all). */
void led_sm_consumer_step(sm_led_ctx_t *ctx, size_t max_events);
/* Compatibility wrapper: delay, run producer step, then consumer step. */
void led_sm_step(sm_led_ctx_t *ctx);
