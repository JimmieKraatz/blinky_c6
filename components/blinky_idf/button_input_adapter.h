#pragma once

#include "blinky_events.h"
#include "blinky_time.h"

typedef struct {
    blinky_event_t (*poll_event)(void *ctx);
    blinky_time_ms_t (*now_ms)(void *ctx);
} button_input_adapter_ops_t;

typedef struct {
    const button_input_adapter_ops_t *ops;
    void *ctx;
} button_input_adapter_t;

static inline blinky_event_t button_input_adapter_poll_event(button_input_adapter_t *adapter)
{
    if (adapter && adapter->ops && adapter->ops->poll_event) {
        return adapter->ops->poll_event(adapter->ctx);
    }
    return BLINKY_EVENT_NONE;
}

static inline blinky_time_ms_t button_input_adapter_now_ms(button_input_adapter_t *adapter)
{
    if (adapter && adapter->ops && adapter->ops->now_ms) {
        return adapter->ops->now_ms(adapter->ctx);
    }
    return 0;
}
