#include "led_event_factory.h"

#include "led_event_map.h"

app_event_t led_event_factory_boot(blinky_time_ms_t now)
{
    return (app_event_t){
        .type = APP_EVENT_BOOT,
        .timestamp_ms = now,
        .payload = {.u32 = 0},
    };
}

app_event_t led_event_factory_from_input(blinky_event_t input, blinky_time_ms_t now)
{
    return (app_event_t){
        .type = led_event_map_from_blinky(input),
        .timestamp_ms = now,
        .payload = {.u32 = 0},
    };
}
