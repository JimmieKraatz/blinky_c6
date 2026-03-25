#pragma once

#include <stdbool.h>

#include "blinky_config_command.h"
#include "blinky_cli_command.h"

/* Parse one CLI line into a command intent.
 * Returns BLINKY_CLI_CMD_NONE for empty/unknown input.
 */
blinky_cli_command_t app_cli_parse_line(const char *line);

/* Parse one CLI line into a config command contract.
 * Returns false for empty/unknown/non-config input.
 */
bool app_cli_parse_config_command(const char *line, blinky_config_command_t *out);
