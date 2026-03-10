#pragma once

#include <stdbool.h>

#include "app_event.h"
#include "blinky_events.h"

/* Convert button semantic events into app event types for dispatching. */
app_event_type_t led_event_map_from_blinky(blinky_event_t ev);

/* Convert app event types into runtime semantic events. */
blinky_event_t led_event_map_to_blinky(app_event_type_t type);

/* Return true when this app event is handled by current runtime consumer path. */
bool led_event_map_is_dispatchable(app_event_type_t type);
