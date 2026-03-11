#include "unity.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

static bool queue_push(void *ctx, const app_event_t *ev)
{
    return app_event_queue_push((app_event_queue_t *)ctx, ev);
}

static const app_event_sink_ops_t QUEUE_SINK_OPS = {
    .push = queue_push,
};

static bool queue_push_and_notify(void *ctx, const app_event_t *ev)
{
    sm_led_ctx_t *sm = (sm_led_ctx_t *)ctx;
    if (!app_event_queue_push(&sm->queue, ev)) {
        return false;
    }
    led_sm_consumer_task_notify(sm);
    return true;
}

static const app_event_sink_ops_t QUEUE_WAKE_SINK_OPS = {
    .push = queue_push_and_notify,
};

static const app_event_source_ops_t QUEUE_SOURCE_OPS = {
    .pop = queue_pop,
    .dropped = queue_dropped,
};

static sm_led_ctx_t g_async_ctx;
static fake_consumer_ctx_t g_async_consumer;

static void reset_async_ctx(void)
{
    g_async_consumer.count = 0;
    g_async_consumer.last_type = APP_EVENT_NONE;
    app_event_queue_init(&g_async_ctx.queue);
    g_async_ctx.sink_ops = &QUEUE_WAKE_SINK_OPS;
    g_async_ctx.sink_ctx = &g_async_ctx;
    app_dispatcher_init(&g_async_ctx.dispatcher,
                        &QUEUE_SOURCE_OPS,
                        &g_async_ctx.queue,
                        fake_consumer_dispatch,
                        &g_async_consumer);
}

static void ensure_async_task_started(void)
{
    /* Ensure deterministic per-test task state regardless of prior test order. */
    led_sm_consumer_task_stop(&g_async_ctx);
    reset_async_ctx();
    led_sm_consumer_task_start(&g_async_ctx);
    g_async_ctx.started = true;
}

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
    ctx.sink_ops = &QUEUE_SINK_OPS;
    ctx.sink_ctx = &ctx.queue;

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
    ctx.sink_ops = &QUEUE_SINK_OPS;
    ctx.sink_ctx = &ctx.queue;

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

TEST_CASE("led sm consumer task notify is safe before task start", "[led_sm_idf]")
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

    led_sm_consumer_task_notify(&ctx);
    vTaskDelay(1);

    TEST_ASSERT_EQUAL_UINT32(0, con.count);
    TEST_ASSERT_EQUAL_UINT32(1, app_event_queue_size(&ctx.queue));
}

TEST_CASE("led sm producer notifies async consumer task", "[led_sm_idf]")
{
    ensure_async_task_started();
    reset_async_ctx();

    fake_input_ctx_t in = {
        .event = BLINKY_EVENT_SHORT_PRESS,
        .now_ms = 88,
    };
    g_async_ctx.input.ops = &(button_input_adapter_ops_t){
        .poll_event = fake_poll_event,
        .now_ms = fake_now_ms,
    };
    g_async_ctx.input.ctx = &in;

    led_sm_producer_step(&g_async_ctx);
    vTaskDelay(1);

    TEST_ASSERT_EQUAL_UINT32(1, g_async_consumer.count);
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, g_async_consumer.last_type);
    TEST_ASSERT_TRUE(app_event_queue_is_empty(&g_async_ctx.queue));
}

TEST_CASE("led sm consumer task drains burst on single notify", "[led_sm_idf]")
{
    ensure_async_task_started();
    reset_async_ctx();

    TEST_ASSERT_TRUE(app_event_queue_push(&g_async_ctx.queue, &(app_event_t){.type = APP_EVENT_TICK}));
    TEST_ASSERT_TRUE(app_event_queue_push(&g_async_ctx.queue, &(app_event_t){.type = APP_EVENT_BUTTON_SHORT}));
    TEST_ASSERT_TRUE(app_event_queue_push(&g_async_ctx.queue, &(app_event_t){.type = APP_EVENT_BUTTON_LONG}));

    led_sm_consumer_task_notify(&g_async_ctx);
    vTaskDelay(1);

    TEST_ASSERT_EQUAL_UINT32(3, g_async_consumer.count);
    TEST_ASSERT_TRUE(app_event_queue_is_empty(&g_async_ctx.queue));
}

TEST_CASE("led sm stop lifecycle is idempotent", "[led_sm_idf]")
{
    ensure_async_task_started();
    reset_async_ctx();

    TEST_ASSERT_TRUE(g_async_ctx.started);
    led_sm_stop(&g_async_ctx);
    TEST_ASSERT_FALSE(g_async_ctx.started);
    led_sm_stop(&g_async_ctx);
    TEST_ASSERT_FALSE(g_async_ctx.started);

    TEST_ASSERT_TRUE(app_event_queue_push(&g_async_ctx.queue, &(app_event_t){.type = APP_EVENT_TICK}));
    led_sm_consumer_task_notify(&g_async_ctx);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_UINT32(0, g_async_consumer.count);
}
