#include "app_event_queue.h"

#include <string.h>

void app_event_queue_init(app_event_queue_t *q)
{
    if (!q) {
        return;
    }
    memset(q, 0, sizeof(*q));
    q->handle = xQueueCreateStatic(
        APP_EVENT_QUEUE_CAPACITY,
        sizeof(app_event_t),
        q->item_storage,
        &q->queue_storage);
}

bool app_event_queue_push(app_event_queue_t *q, const app_event_t *ev)
{
    if (!q || !q->handle || !ev) {
        return false;
    }

    if (xQueueSendToBack(q->handle, ev, 0) != pdPASS) {
        q->dropped++;
        return false;
    }
    return true;
}

bool app_event_queue_pop(app_event_queue_t *q, app_event_t *out)
{
    if (!q || !q->handle || !out) {
        return false;
    }

    return (xQueueReceive(q->handle, out, 0) == pdPASS);
}

bool app_event_queue_is_empty(const app_event_queue_t *q)
{
    return (app_event_queue_size(q) == 0U);
}

size_t app_event_queue_size(const app_event_queue_t *q)
{
    if (!q || !q->handle) {
        return 0U;
    }
    return (size_t)uxQueueMessagesWaiting(q->handle);
}

uint32_t app_event_queue_dropped(const app_event_queue_t *q)
{
    if (!q) {
        return 0U;
    }
    return q->dropped;
}
