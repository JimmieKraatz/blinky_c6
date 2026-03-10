#include "led_event_consumer.h"

#include <string.h>

#include "led_event_map.h"

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
    if (!led_event_map_is_dispatchable(ev->type)) {
        return;
    }

    led_runtime_step(rt, ev->timestamp_ms, led_event_map_to_blinky(ev->type), out);
}
