#include "unity.h"

#include "app_dispatcher.h"

typedef struct {
    app_event_t items[4];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint32_t dropped;
} fake_source_t;

typedef struct {
    uint32_t calls;
    app_event_type_t last_type;
} fake_consumer_t;

static bool fake_pop(void *ctx, app_event_t *out)
{
    fake_source_t *s = (fake_source_t *)ctx;
    if (!s || !out || s->count == 0U) {
        return false;
    }
    *out = s->items[s->head];
    s->head = (uint8_t)((s->head + 1U) % 4U);
    s->count--;
    return true;
}

static uint32_t fake_dropped(void *ctx)
{
    fake_source_t *s = (fake_source_t *)ctx;
    return s ? s->dropped : 0U;
}

static void fake_consume(void *ctx, const app_event_t *ev)
{
    fake_consumer_t *c = (fake_consumer_t *)ctx;
    if (!c || !ev) {
        return;
    }
    c->calls++;
    c->last_type = ev->type;
}

static void fake_push(fake_source_t *s, app_event_type_t type)
{
    s->items[s->tail] = (app_event_t){
        .type = type,
        .timestamp_ms = s->tail,
        .payload = {.u32 = s->tail},
    };
    s->tail = (uint8_t)((s->tail + 1U) % 4U);
    s->count++;
}

TEST_CASE("dispatcher run_once dispatches one event", "[app_dispatcher]")
{
    fake_source_t src = {0};
    fake_consumer_t con = {0};
    app_dispatcher_t d = {0};

    fake_push(&src, APP_EVENT_BOOT);
    app_dispatcher_init(&d,
                        &(app_event_source_ops_t){.pop = fake_pop, .dropped = fake_dropped},
                        &src,
                        fake_consume,
                        &con);

    TEST_ASSERT_TRUE(app_dispatcher_run_once(&d));
    TEST_ASSERT_EQUAL_UINT32(1, con.calls);
    TEST_ASSERT_EQUAL(APP_EVENT_BOOT, con.last_type);
    TEST_ASSERT_EQUAL_UINT32(1, app_dispatcher_dispatched(&d));
}

TEST_CASE("dispatcher drain drains all when max is zero", "[app_dispatcher]")
{
    fake_source_t src = {0};
    fake_consumer_t con = {0};
    app_dispatcher_t d = {0};

    fake_push(&src, APP_EVENT_BOOT);
    fake_push(&src, APP_EVENT_TICK);
    fake_push(&src, APP_EVENT_BUTTON_SHORT);

    app_dispatcher_init(&d,
                        &(app_event_source_ops_t){.pop = fake_pop, .dropped = fake_dropped},
                        &src,
                        fake_consume,
                        &con);

    TEST_ASSERT_EQUAL_UINT32(3, app_dispatcher_drain(&d, 0));
    TEST_ASSERT_EQUAL_UINT32(3, con.calls);
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, con.last_type);
}

TEST_CASE("dispatcher drain honors max_events bound", "[app_dispatcher]")
{
    fake_source_t src = {0};
    fake_consumer_t con = {0};
    app_dispatcher_t d = {0};

    fake_push(&src, APP_EVENT_BOOT);
    fake_push(&src, APP_EVENT_TICK);
    fake_push(&src, APP_EVENT_BUTTON_LONG);

    app_dispatcher_init(&d,
                        &(app_event_source_ops_t){.pop = fake_pop, .dropped = fake_dropped},
                        &src,
                        fake_consume,
                        &con);

    TEST_ASSERT_EQUAL_UINT32(2, app_dispatcher_drain(&d, 2));
    TEST_ASSERT_EQUAL_UINT32(2, con.calls);
    TEST_ASSERT_EQUAL(APP_EVENT_TICK, con.last_type);
}

TEST_CASE("dispatcher dropped count proxies source dropped metric", "[app_dispatcher]")
{
    fake_source_t src = {.dropped = 7};
    fake_consumer_t con = {0};
    app_dispatcher_t d = {0};

    app_dispatcher_init(&d,
                        &(app_event_source_ops_t){.pop = fake_pop, .dropped = fake_dropped},
                        &src,
                        fake_consume,
                        &con);

    TEST_ASSERT_EQUAL_UINT32(7, app_dispatcher_dropped(&d));
}

TEST_CASE("dispatcher null safety", "[app_dispatcher]")
{
    app_dispatcher_t d = {0};
    TEST_ASSERT_FALSE(app_dispatcher_run_once(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, app_dispatcher_drain(NULL, 0));
    TEST_ASSERT_EQUAL_UINT32(0, app_dispatcher_dispatched(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, app_dispatcher_dropped(NULL));

    app_dispatcher_init(&d, NULL, NULL, NULL, NULL);
    TEST_ASSERT_FALSE(app_dispatcher_run_once(&d));
}
