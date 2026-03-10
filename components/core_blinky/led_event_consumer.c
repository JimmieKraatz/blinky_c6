#include "led_event_consumer.h"

#include <string.h>

static blinky_event_t to_blinky_event(app_event_type_t type)
{
    switch (type) {
    case APP_EVENT_BUTTON_SHORT:
        return BLINKY_EVENT_SHORT_PRESS;
    case APP_EVENT_BUTTON_LONG:
        return BLINKY_EVENT_LONG_PRESS;
    case APP_EVENT_TICK:
    case APP_EVENT_BOOT:
        return BLINKY_EVENT_NONE;
    default:
        return BLINKY_EVENT_NONE;
    }
}

static bool should_dispatch(app_event_type_t type)
{
    switch (type) {
    case APP_EVENT_BOOT:
    case APP_EVENT_TICK:
    case APP_EVENT_BUTTON_SHORT:
    case APP_EVENT_BUTTON_LONG:
        return true;
    default:
        return false;
    }
}

void led_event_consumer_dispatch(led_runtime_t *rt,
                                 const app_event_t *ev,
                                 led_runtime_output_t *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!rt || !ev || !out) {
        return;
    }
    if (!should_dispatch(ev->type)) {
        return;
    }

    led_runtime_step(rt, ev->timestamp_ms, to_blinky_event(ev->type), out);
}
