#include "led_event_consumer.h"

#include <assert.h>
#include <string.h>

#include "led_event_map.h"

led_command_result_t led_event_consumer_dispatch(led_runtime_t *rt,
                                                 const app_event_t *ev,
                                                 led_runtime_output_t *out)
{
    assert(rt);
    assert(ev);
    assert(out);

    memset(out, 0, sizeof(*out));
    if (ev->type == APP_EVENT_BLINKY_COMMAND) {
        return led_command_dispatch(rt, (blinky_control_command_t)ev->payload.u32, ev->timestamp_ms, out);
    }

    if (!led_event_map_is_dispatchable(ev->type)) {
        return LED_COMMAND_RESULT_INVALID;
    }

    led_runtime_step(rt, ev->timestamp_ms, led_event_map_to_blinky(ev->type), out);
    return LED_COMMAND_RESULT_APPLIED;
}
