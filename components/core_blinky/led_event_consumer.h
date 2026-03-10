#pragma once

#include "app_event.h"
#include "led_runtime.h"

/* Dispatch one semantic app event into LED runtime orchestration. */
void led_event_consumer_dispatch(led_runtime_t *rt,
                                 const app_event_t *ev,
                                 led_runtime_output_t *out);
