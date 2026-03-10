#pragma once

#include <stdint.h>

#include "blinky_time.h"

typedef enum {
    APP_EVENT_NONE = 0,
    APP_EVENT_BOOT,
    APP_EVENT_TICK,
    APP_EVENT_BUTTON_SHORT,
    APP_EVENT_BUTTON_LONG,
    APP_EVENT_MENU_TIMEOUT,
    APP_EVENT_MODEL_STEP_DUE,
    APP_EVENT_FAULT,
    APP_EVENT_SHUTDOWN,
} app_event_type_t;

typedef union {
    uint32_t u32;
    int32_t i32;
} app_event_payload_t;

typedef struct {
    app_event_type_t type;
    blinky_time_ms_t timestamp_ms;
    app_event_payload_t payload;
} app_event_t;
