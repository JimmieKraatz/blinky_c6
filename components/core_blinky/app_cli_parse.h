#pragma once

#include "blinky_cli_command.h"

/* Parse one CLI line into a command intent.
 * Returns BLINKY_CLI_CMD_NONE for empty/unknown input.
 */
blinky_cli_command_t app_cli_parse_line(const char *line);
