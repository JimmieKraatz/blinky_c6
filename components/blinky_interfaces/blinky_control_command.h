#pragma once

/* Semantic blinky-domain control commands.
 * These are higher-level than raw button input and are intended to be
 * interpreted by the LED domain against current runtime state.
 */
typedef enum {
    BLINKY_CONTROL_CMD_NONE = 0,
    BLINKY_CONTROL_CMD_RUN,
    BLINKY_CONTROL_CMD_PAUSE,
    BLINKY_CONTROL_CMD_RUN_PAUSE_TOGGLE,
    BLINKY_CONTROL_CMD_MENU_ENTER,
    BLINKY_CONTROL_CMD_MENU_NEXT,
    BLINKY_CONTROL_CMD_MENU_EXIT,
} blinky_control_command_t;
