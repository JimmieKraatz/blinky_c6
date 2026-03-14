#include "app_cli_command_map.h"

app_event_type_t app_cli_command_map_to_app_event(blinky_cli_command_t cmd)
{
    switch (cmd) {
    case BLINKY_CLI_CMD_RUN:
    case BLINKY_CLI_CMD_PAUSE:
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

bool app_cli_command_map_is_dispatchable(blinky_cli_command_t cmd)
{
    return app_cli_command_map_to_app_event(cmd) != APP_EVENT_NONE;
}

bool app_cli_command_map_is_allowed_in_state(blinky_cli_command_t cmd, led_policy_state_t state)
{
    switch (cmd) {
    case BLINKY_CLI_CMD_RUN:
        return state == LED_POLICY_PAUSED;
    case BLINKY_CLI_CMD_PAUSE:
        return state == LED_POLICY_RUNNING;
    case BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE:
        return state == LED_POLICY_RUNNING || state == LED_POLICY_PAUSED;
    case BLINKY_CLI_CMD_MENU_ENTER:
        return state == LED_POLICY_RUNNING || state == LED_POLICY_PAUSED;
    case BLINKY_CLI_CMD_MENU_NEXT:
    case BLINKY_CLI_CMD_MENU_EXIT:
        return state == LED_POLICY_MENU;
    case BLINKY_CLI_CMD_NONE:
    case BLINKY_CLI_CMD_HELP:
    case BLINKY_CLI_CMD_STATUS:
    default:
        return false;
    }
}
