#include "app_dispatcher.h"

#include <string.h>

void app_dispatcher_init(app_dispatcher_t *d,
                         const app_event_source_ops_t *source_ops,
                         void *source_ctx,
                         app_event_consumer_fn consumer,
                         void *consumer_ctx)
{
    if (!d) {
        return;
    }

    memset(d, 0, sizeof(*d));
    d->source_ops = source_ops;
    d->source_ctx = source_ctx;
    d->consumer = consumer;
    d->consumer_ctx = consumer_ctx;
}

bool app_dispatcher_run_once(app_dispatcher_t *d)
{
    app_event_t ev = {0};
    if (!d || !d->source_ops || !d->source_ops->pop || !d->consumer) {
        return false;
    }
    if (!d->source_ops->pop(d->source_ctx, &ev)) {
        return false;
    }

    d->consumer(d->consumer_ctx, &ev);
    d->dispatched++;
    return true;
}

size_t app_dispatcher_drain(app_dispatcher_t *d, size_t max_events)
{
    size_t count = 0;
    if (!d) {
        return 0;
    }

    if (max_events == 0U) {
        while (app_dispatcher_run_once(d)) {
            count++;
        }
        return count;
    }

    while (count < max_events && app_dispatcher_run_once(d)) {
        count++;
    }
    return count;
}

uint32_t app_dispatcher_dispatched(const app_dispatcher_t *d)
{
    if (!d) {
        return 0U;
    }
    return d->dispatched;
}

uint32_t app_dispatcher_dropped(const app_dispatcher_t *d)
{
    if (!d || !d->source_ops || !d->source_ops->dropped) {
        return 0U;
    }
    return d->source_ops->dropped(d->source_ctx);
}
