#pragma once

#include <stdbool.h>

#include "app_event.h"
#include "blinky_cli_command.h"
#include "blinky_control_command.h"

/* Map CLI command intent to semantic blinky-domain control command.
 * Returns false for non-blinky commands (for example help/status).
 */
bool app_cli_command_map_to_blinky_command(blinky_cli_command_t cmd, blinky_control_command_t *out);

/* Return true when a CLI command should enqueue into runtime dispatch path. */
bool app_cli_command_map_is_dispatchable(blinky_cli_command_t cmd);
