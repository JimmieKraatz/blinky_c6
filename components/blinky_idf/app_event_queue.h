#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_event.h"

#ifndef APP_EVENT_QUEUE_CAPACITY
#define APP_EVENT_QUEUE_CAPACITY 16U
#endif

typedef struct {
    app_event_t items[APP_EVENT_QUEUE_CAPACITY];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint32_t dropped;
} app_event_queue_t;

void app_event_queue_init(app_event_queue_t *q);
bool app_event_queue_push(app_event_queue_t *q, const app_event_t *ev);
bool app_event_queue_pop(app_event_queue_t *q, app_event_t *out);
bool app_event_queue_is_empty(const app_event_queue_t *q);
size_t app_event_queue_size(const app_event_queue_t *q);
uint32_t app_event_queue_dropped(const app_event_queue_t *q);
