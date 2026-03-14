#include "app_cli_command_map.h"

bool app_cli_command_map_to_blinky_command(blinky_cli_command_t cmd, blinky_control_command_t *out)
{
    if (!out) {
        return false;
    }

    switch (cmd) {
    case BLINKY_CLI_CMD_RUN:
        *out = BLINKY_CONTROL_CMD_RUN;
        return true;
    case BLINKY_CLI_CMD_PAUSE:
        *out = BLINKY_CONTROL_CMD_PAUSE;
        return true;
    case BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE:
        *out = BLINKY_CONTROL_CMD_RUN_PAUSE_TOGGLE;
        return true;
    case BLINKY_CLI_CMD_MENU_ENTER:
        *out = BLINKY_CONTROL_CMD_MENU_ENTER;
        return true;
    case BLINKY_CLI_CMD_MENU_NEXT:
        *out = BLINKY_CONTROL_CMD_MENU_NEXT;
        return true;
    case BLINKY_CLI_CMD_MENU_EXIT:
        *out = BLINKY_CONTROL_CMD_MENU_EXIT;
        return true;
    case BLINKY_CLI_CMD_NONE:
    case BLINKY_CLI_CMD_HELP:
    case BLINKY_CLI_CMD_STATUS:
    default:
        *out = BLINKY_CONTROL_CMD_NONE;
        return false;
    }
}

bool app_cli_command_map_is_dispatchable(blinky_cli_command_t cmd)
{
    blinky_control_command_t out = BLINKY_CONTROL_CMD_NONE;
    return app_cli_command_map_to_blinky_command(cmd, &out);
}
