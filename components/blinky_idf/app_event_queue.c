#include "app_event_queue.h"

#include <string.h>

void app_event_queue_init(app_event_queue_t *q)
{
    if (!q) {
        return;
    }
    memset(q, 0, sizeof(*q));
}

bool app_event_queue_push(app_event_queue_t *q, const app_event_t *ev)
{
    if (!q || !ev) {
        return false;
    }
    if (q->count >= APP_EVENT_QUEUE_CAPACITY) {
        q->dropped++;
        return false;
    }

    q->items[q->tail] = *ev;
    q->tail = (uint8_t)((q->tail + 1U) % APP_EVENT_QUEUE_CAPACITY);
    q->count++;
    return true;
}

bool app_event_queue_pop(app_event_queue_t *q, app_event_t *out)
{
    if (!q || !out || q->count == 0U) {
        return false;
    }

    *out = q->items[q->head];
    q->head = (uint8_t)((q->head + 1U) % APP_EVENT_QUEUE_CAPACITY);
    q->count--;
    return true;
}

bool app_event_queue_is_empty(const app_event_queue_t *q)
{
    return (!q || q->count == 0U);
}

size_t app_event_queue_size(const app_event_queue_t *q)
{
    if (!q) {
        return 0U;
    }
    return q->count;
}

uint32_t app_event_queue_dropped(const app_event_queue_t *q)
{
    if (!q) {
        return 0U;
    }
    return q->dropped;
}
