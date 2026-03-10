#include "unity.h"

#include "led_sm_idf.h"

typedef struct {
    blinky_event_t event;
    blinky_time_ms_t now_ms;
    int poll_calls;
    int now_calls;
} fake_input_ctx_t;

static blinky_event_t fake_poll_event(void *ctx)
{
    fake_input_ctx_t *f = (fake_input_ctx_t *)ctx;
    f->poll_calls++;
    return f->event;
}

static blinky_time_ms_t fake_now_ms(void *ctx)
{
    fake_input_ctx_t *f = (fake_input_ctx_t *)ctx;
    f->now_calls++;
    return f->now_ms;
}

typedef struct {
    uint32_t count;
    app_event_type_t last_type;
} fake_consumer_ctx_t;

static void fake_consumer_dispatch(void *ctx, const app_event_t *ev)
{
    fake_consumer_ctx_t *f = (fake_consumer_ctx_t *)ctx;
    f->count++;
    f->last_type = ev->type;
}

static bool queue_pop(void *ctx, app_event_t *out)
{
    return app_event_queue_pop((app_event_queue_t *)ctx, out);
}

static uint32_t queue_dropped(void *ctx)
{
    return app_event_queue_dropped((app_event_queue_t *)ctx);
}

static const app_event_source_ops_t QUEUE_SOURCE_OPS = {
    .pop = queue_pop,
    .dropped = queue_dropped,
};

TEST_CASE("led sm producer maps short press to app event", "[led_sm_idf]")
{
    sm_led_ctx_t ctx = {0};
    fake_input_ctx_t in = {
        .event = BLINKY_EVENT_SHORT_PRESS,
        .now_ms = 1234,
    };

    ctx.input.ops = &(button_input_adapter_ops_t){
        .poll_event = fake_poll_event,
        .now_ms = fake_now_ms,
    };
    ctx.input.ctx = &in;
    app_event_queue_init(&ctx.queue);

    led_sm_producer_step(&ctx);

    app_event_t out = {0};
    TEST_ASSERT_TRUE(app_event_queue_pop(&ctx.queue, &out));
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, out.type);
    TEST_ASSERT_EQUAL_UINT32(1234, out.timestamp_ms);
    TEST_ASSERT_EQUAL_INT(1, in.poll_calls);
    TEST_ASSERT_EQUAL_INT(1, in.now_calls);
}

TEST_CASE("led sm producer maps no event to tick", "[led_sm_idf]")
{
    sm_led_ctx_t ctx = {0};
    fake_input_ctx_t in = {
        .event = BLINKY_EVENT_NONE,
        .now_ms = 77,
    };

    ctx.input.ops = &(button_input_adapter_ops_t){
        .poll_event = fake_poll_event,
        .now_ms = fake_now_ms,
    };
    ctx.input.ctx = &in;
    app_event_queue_init(&ctx.queue);

    led_sm_producer_step(&ctx);

    app_event_t out = {0};
    TEST_ASSERT_TRUE(app_event_queue_pop(&ctx.queue, &out));
    TEST_ASSERT_EQUAL(APP_EVENT_TICK, out.type);
    TEST_ASSERT_EQUAL_UINT32(77, out.timestamp_ms);
}

TEST_CASE("led sm consumer honors max_events bound", "[led_sm_idf]")
{
    sm_led_ctx_t ctx = {0};
    fake_consumer_ctx_t con = {0};

    app_event_queue_init(&ctx.queue);
    app_dispatcher_init(&ctx.dispatcher,
                        &QUEUE_SOURCE_OPS,
                        &ctx.queue,
                        fake_consumer_dispatch,
                        &con);

    TEST_ASSERT_TRUE(app_event_queue_push(&ctx.queue, &(app_event_t){.type = APP_EVENT_TICK}));
    TEST_ASSERT_TRUE(app_event_queue_push(&ctx.queue, &(app_event_t){.type = APP_EVENT_BUTTON_SHORT}));
    TEST_ASSERT_TRUE(app_event_queue_push(&ctx.queue, &(app_event_t){.type = APP_EVENT_BUTTON_LONG}));

    led_sm_consumer_step(&ctx, 2);

    TEST_ASSERT_EQUAL_UINT32(2, con.count);
    TEST_ASSERT_EQUAL_UINT32(2, app_dispatcher_dispatched(&ctx.dispatcher));
    TEST_ASSERT_EQUAL_UINT32(1, app_event_queue_size(&ctx.queue));
}

TEST_CASE("led sm consumer drains all when max_events is zero", "[led_sm_idf]")
{
    sm_led_ctx_t ctx = {0};
    fake_consumer_ctx_t con = {0};

    app_event_queue_init(&ctx.queue);
    app_dispatcher_init(&ctx.dispatcher,
                        &QUEUE_SOURCE_OPS,
                        &ctx.queue,
                        fake_consumer_dispatch,
                        &con);

    TEST_ASSERT_TRUE(app_event_queue_push(&ctx.queue, &(app_event_t){.type = APP_EVENT_TICK}));
    TEST_ASSERT_TRUE(app_event_queue_push(&ctx.queue, &(app_event_t){.type = APP_EVENT_BUTTON_SHORT}));

    led_sm_consumer_step(&ctx, 0);

    TEST_ASSERT_EQUAL_UINT32(2, con.count);
    TEST_ASSERT_EQUAL_UINT32(2, app_dispatcher_dispatched(&ctx.dispatcher));
    TEST_ASSERT_TRUE(app_event_queue_is_empty(&ctx.queue));
}
