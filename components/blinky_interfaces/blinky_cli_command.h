#pragma once

/* Transport-agnostic CLI command intents.
 * Platform adapters parse text and map into these command values.
 */
typedef enum {
    BLINKY_CLI_CMD_NONE = 0,
    BLINKY_CLI_CMD_HELP,
    BLINKY_CLI_CMD_STATUS,
    BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE,
    BLINKY_CLI_CMD_MENU_ENTER,
    BLINKY_CLI_CMD_MENU_NEXT,
    BLINKY_CLI_CMD_MENU_EXIT,
} blinky_cli_command_t;

