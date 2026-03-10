#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_event.h"

typedef bool (*app_event_pop_fn)(void *ctx, app_event_t *out);
typedef uint32_t (*app_event_dropped_fn)(void *ctx);
typedef bool (*app_event_push_fn)(void *ctx, const app_event_t *ev);
typedef void (*app_event_consumer_fn)(void *ctx, const app_event_t *ev);

typedef struct {
    app_event_push_fn push;
} app_event_sink_ops_t;

typedef struct {
    app_event_pop_fn pop;
    app_event_dropped_fn dropped;
} app_event_source_ops_t;

typedef struct {
    const app_event_source_ops_t *source_ops;
    void *source_ctx;
    app_event_consumer_fn consumer;
    void *consumer_ctx;
    uint32_t dispatched;
} app_dispatcher_t;

void app_dispatcher_init(app_dispatcher_t *d,
                         const app_event_source_ops_t *source_ops,
                         void *source_ctx,
                         app_event_consumer_fn consumer,
                         void *consumer_ctx);

bool app_dispatcher_run_once(app_dispatcher_t *d);
size_t app_dispatcher_drain(app_dispatcher_t *d, size_t max_events);
uint32_t app_dispatcher_dispatched(const app_dispatcher_t *d);
uint32_t app_dispatcher_dropped(const app_dispatcher_t *d);
