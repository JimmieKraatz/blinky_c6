#include "led_cli_command_map.h"

app_event_type_t led_cli_command_map_to_app_event(blinky_cli_command_t cmd)
{
    switch (cmd) {
    case BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE:
        return APP_EVENT_BUTTON_SHORT;
    case BLINKY_CLI_CMD_MENU_ENTER:
    case BLINKY_CLI_CMD_MENU_EXIT:
        return APP_EVENT_BUTTON_LONG;
    case BLINKY_CLI_CMD_MENU_NEXT:
        return APP_EVENT_BUTTON_SHORT;
    case BLINKY_CLI_CMD_NONE:
    case BLINKY_CLI_CMD_HELP:
    case BLINKY_CLI_CMD_STATUS:
    default:
        return APP_EVENT_NONE;
    }
}

bool led_cli_command_map_is_dispatchable(blinky_cli_command_t cmd)
{
    return led_cli_command_map_to_app_event(cmd) != APP_EVENT_NONE;
}

