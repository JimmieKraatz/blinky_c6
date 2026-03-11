#pragma once

#include "app_event.h"
#include "led_runtime.h"

/* Dispatch one semantic app event into LED runtime orchestration.
 * Contract: rt, ev, and out are required (non-null).
 */
void led_event_consumer_dispatch(led_runtime_t *rt,
                                 const app_event_t *ev,
                                 led_runtime_output_t *out);
