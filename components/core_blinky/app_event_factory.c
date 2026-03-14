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
    blinky_control_command_t blinky_cmd = BLINKY_CONTROL_CMD_NONE;
    if (!app_cli_command_map_to_blinky_command(cmd, &blinky_cmd)) {
        return (app_event_t){
            .type = APP_EVENT_NONE,
            .timestamp_ms = now,
            .payload = {.u32 = 0},
        };
    }

    return app_event_factory_from_blinky_command(blinky_cmd, now);
}

app_event_t app_event_factory_from_blinky_command(blinky_control_command_t cmd, blinky_time_ms_t now)
{
    return (app_event_t){
        .type = cmd == BLINKY_CONTROL_CMD_NONE ? APP_EVENT_NONE : APP_EVENT_BLINKY_COMMAND,
        .timestamp_ms = now,
        .payload = {.u32 = (uint32_t)cmd},
    };
}
