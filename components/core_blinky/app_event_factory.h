#pragma once

#include "app_event.h"
#include "blinky_cli_command.h"
#include "blinky_events.h"
#include "blinky_time.h"

/* Build a boot semantic app event with core-owned default payload policy. */
app_event_t app_event_factory_boot(blinky_time_ms_t now);

/* Build one semantic app event from input event + timestamp. */
app_event_t app_event_factory_from_input(blinky_event_t input, blinky_time_ms_t now);

/* Build one semantic app event from CLI command intent + timestamp. */
app_event_t app_event_factory_from_cli_command(blinky_cli_command_t cmd, blinky_time_ms_t now);
