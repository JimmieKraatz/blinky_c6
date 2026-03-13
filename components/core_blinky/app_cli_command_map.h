#pragma once

#include <stdbool.h>

#include "app_event.h"
#include "blinky_cli_command.h"

/* Map CLI command intent to existing app event contract.
 * Returns APP_EVENT_NONE for non-dispatch commands (for example help/status).
 */
app_event_type_t app_cli_command_map_to_app_event(blinky_cli_command_t cmd);

/* Return true when a CLI command should enqueue into runtime dispatch path. */
bool app_cli_command_map_is_dispatchable(blinky_cli_command_t cmd);

