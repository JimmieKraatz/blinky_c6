#include "app_event_factory.h"

#include "app_cli_command_map.h"
#include "led_event_map.h"

app_event_t app_event_factory_boot(blinky_time_ms_t now)
{
    return (app_event_t){
        .type = APP_EVENT_BOOT,
        .timestamp_ms = now,
        .payload = {.u32 = 0},
    };
}

app_event_t app_event_factory_from_input(blinky_event_t input, blinky_time_ms_t now)
{
    return (app_event_t){
        .type = led_event_map_from_blinky(input),
        .timestamp_ms = now,
        .payload = {.u32 = 0},
    };
}

app_event_t app_event_factory_from_cli_command(blinky_cli_command_t cmd, blinky_time_ms_t now)
{
    return (app_event_t){
        .type = app_cli_command_map_to_app_event(cmd),
        .timestamp_ms = now,
        .payload = {.u32 = 0},
    };
}
