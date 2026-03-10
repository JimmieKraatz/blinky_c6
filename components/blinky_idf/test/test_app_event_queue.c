#include "unity.h"

#include "app_event_queue.h"

static app_event_t mk_event(app_event_type_t type, blinky_time_ms_t ts)
{
    app_event_t ev = {
        .type = type,
        .timestamp_ms = ts,
        .payload = { .u32 = ts },
    };
    return ev;
}

TEST_CASE("app event queue push/pop preserves FIFO order", "[app_event_queue]")
{
    app_event_queue_t q = {0};
    app_event_t out = {0};

    app_event_queue_init(&q);
    TEST_ASSERT_TRUE(app_event_queue_is_empty(&q));

    TEST_ASSERT_TRUE(app_event_queue_push(&q, &(app_event_t){
        .type = APP_EVENT_BOOT, .timestamp_ms = 1, .payload = {.u32 = 11}}));
    TEST_ASSERT_TRUE(app_event_queue_push(&q, &(app_event_t){
        .type = APP_EVENT_TICK, .timestamp_ms = 2, .payload = {.u32 = 22}}));
    TEST_ASSERT_TRUE(app_event_queue_push(&q, &(app_event_t){
        .type = APP_EVENT_BUTTON_SHORT, .timestamp_ms = 3, .payload = {.u32 = 33}}));

    TEST_ASSERT_EQUAL_UINT32(3, app_event_queue_size(&q));

    TEST_ASSERT_TRUE(app_event_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL(APP_EVENT_BOOT, out.type);
    TEST_ASSERT_EQUAL_UINT32(1, out.timestamp_ms);

    TEST_ASSERT_TRUE(app_event_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL(APP_EVENT_TICK, out.type);
    TEST_ASSERT_EQUAL_UINT32(2, out.timestamp_ms);

    TEST_ASSERT_TRUE(app_event_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, out.type);
    TEST_ASSERT_EQUAL_UINT32(3, out.timestamp_ms);

    TEST_ASSERT_TRUE(app_event_queue_is_empty(&q));
}

TEST_CASE("app event queue tracks dropped count on overflow", "[app_event_queue]")
{
    app_event_queue_t q = {0};
    app_event_t out = {0};
    app_event_t ev = mk_event(APP_EVENT_TICK, 0);

    app_event_queue_init(&q);
    for (uint32_t i = 0; i < APP_EVENT_QUEUE_CAPACITY; ++i) {
        ev.timestamp_ms = i;
        TEST_ASSERT_TRUE(app_event_queue_push(&q, &ev));
    }

    ev.timestamp_ms = 1234;
    TEST_ASSERT_FALSE(app_event_queue_push(&q, &ev));
    TEST_ASSERT_EQUAL_UINT32(1, app_event_queue_dropped(&q));
    TEST_ASSERT_EQUAL_UINT32(APP_EVENT_QUEUE_CAPACITY, app_event_queue_size(&q));

    TEST_ASSERT_TRUE(app_event_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(0, out.timestamp_ms);
}

TEST_CASE("app event queue null and empty safety", "[app_event_queue]")
{
    app_event_queue_t q = {0};
    app_event_t ev = mk_event(APP_EVENT_BOOT, 7);
    app_event_t out = {0};

    app_event_queue_init(&q);

    TEST_ASSERT_FALSE(app_event_queue_push(NULL, &ev));
    TEST_ASSERT_FALSE(app_event_queue_push(&q, NULL));
    TEST_ASSERT_FALSE(app_event_queue_pop(&q, &out));
    TEST_ASSERT_FALSE(app_event_queue_pop(NULL, &out));
    TEST_ASSERT_FALSE(app_event_queue_pop(&q, NULL));

    TEST_ASSERT_TRUE(app_event_queue_is_empty(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, app_event_queue_size(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, app_event_queue_dropped(NULL));
}
