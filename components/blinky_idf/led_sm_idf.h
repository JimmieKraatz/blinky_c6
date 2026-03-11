#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_dispatcher.h"
#include "app_event_queue.h"
#include "button_input_adapter.h"
#include "button_input_adapter_idf.h"
#include "blinky_log.h"
#include "blinky_log_adapter_idf.h"
#include "led_runtime.h"
#include "led_output_adapter.h"
#include "led_output_adapter_idf.h"
#include "led_config_idf.h"

/* LED state machine context.
 * Owns runtime state for LED behavior and local button debounce.
 */
typedef struct {
    button_input_adapter_t input;
    button_input_adapter_idf_t input_idf;

    led_output_adapter_t led_output;
    led_output_adapter_idf_t led_output_idf;
    blinky_log_sink_t log_sink;
    blinky_log_adapter_idf_t log_idf;
    led_runtime_t runtime;
    led_platform_config_t platform_cfg;
    led_core_config_t core_cfg;
    app_event_queue_t queue;
    const app_event_sink_ops_t *sink_ops;
    void *sink_ctx;
    app_dispatcher_t dispatcher;
    bool started;
} sm_led_ctx_t;

/* Initialize LED hardware and enter initial LED FSM state. */
void led_sm_init(sm_led_ctx_t *ctx);
/* Explicit lifecycle boundaries for async consumer path. */
void led_sm_start(sm_led_ctx_t *ctx);
void led_sm_stop(sm_led_ctx_t *ctx);

/* Producer side: sample inputs and enqueue one semantic app event. */
void led_sm_producer_step(sm_led_ctx_t *ctx);
/* Push one app event through configured sink boundary. */
bool led_sm_enqueue_event(sm_led_ctx_t *ctx, const app_event_t *ev);
/* Consumer side: dispatch queued events (0 => drain all). */
void led_sm_consumer_step(sm_led_ctx_t *ctx, size_t max_events);
/* Start background consumer task and provide event wake-up signal. */
void led_sm_consumer_task_start(sm_led_ctx_t *ctx);
void led_sm_consumer_task_stop(sm_led_ctx_t *ctx);
void led_sm_consumer_task_notify(sm_led_ctx_t *ctx);
/* Compatibility wrapper: delay, run producer step, then consumer step. */
void led_sm_step(sm_led_ctx_t *ctx);
