#include "button_input_adapter_idf.h"

#include "freertos/task.h"

static inline blinky_event_t to_blinky_event(button_event_t ev)
{
    switch (ev) {
    case BUTTON_EVENT_SHORT_PRESS:
        return BLINKY_EVENT_SHORT_PRESS;
    case BUTTON_EVENT_LONG_PRESS:
        return BLINKY_EVENT_LONG_PRESS;
    case BUTTON_EVENT_NONE:
    default:
        return BLINKY_EVENT_NONE;
    }
}

static blinky_event_t idf_poll_event(void *ctx)
{
    button_input_adapter_idf_t *impl = (button_input_adapter_idf_t *)ctx;
    button_event_t ev = button_poll_event(&impl->button, xTaskGetTickCount());
    return to_blinky_event(ev);
}

static blinky_time_ms_t idf_now_ms(void *ctx)
{
    (void)ctx;
    return (blinky_time_ms_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static const button_input_adapter_ops_t IDF_OPS = {
    .poll_event = idf_poll_event,
    .now_ms = idf_now_ms,
};

void button_input_adapter_idf_init(button_input_adapter_t *adapter,
                                   button_input_adapter_idf_t *impl,
                                   const button_input_adapter_idf_config_t *cfg)
{
    if (!adapter || !impl || !cfg) {
        return;
    }

    button_init(&impl->button,
                cfg->gpio,
                cfg->active_low,
                cfg->pull,
                cfg->timing.debounce_count,
                cfg->timing.long_press_ms);

    adapter->ops = &IDF_OPS;
    adapter->ctx = impl;
}
