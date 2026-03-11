#include "blinky_log.h"

#include "unity.h"

typedef struct {
    uint32_t calls;
    const blinky_log_record_t *last_record;
} fake_log_ctx_t;

static void fake_emit(void *ctx, const blinky_log_record_t *record)
{
    fake_log_ctx_t *state = (fake_log_ctx_t *)ctx;
    state->calls += 1;
    state->last_record = record;
}

TEST_CASE("log sink forwards structured record", "[blinky_log]")
{
    fake_log_ctx_t ctx = {0};
    const blinky_log_sink_ops_t ops = {
        .emit = fake_emit,
    };
    blinky_log_sink_t sink = {
        .ops = &ops,
        .ctx = &ctx,
    };

    const blinky_log_kv_t kvs[] = {
        blinky_log_kv_uint("percent", 42),
        blinky_log_kv_bool("menu", true),
    };
    const blinky_log_record_t record = {
        .level = BLINKY_LOG_LEVEL_INFO,
        .domain = "runtime",
        .event = "state_change",
        .message = "entered paused",
        .kvs = kvs,
        .kv_count = 2,
    };

    blinky_log_emit(&sink, &record);

    TEST_ASSERT_EQUAL_UINT32(1, ctx.calls);
    TEST_ASSERT_NOT_NULL(ctx.last_record);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_INFO, (uint32_t)ctx.last_record->level);
    TEST_ASSERT_EQUAL_STRING("runtime", ctx.last_record->domain);
    TEST_ASSERT_EQUAL_STRING("state_change", ctx.last_record->event);
    TEST_ASSERT_EQUAL_STRING("entered paused", ctx.last_record->message);
    TEST_ASSERT_EQUAL_UINT32(2, ctx.last_record->kv_count);
    TEST_ASSERT_EQUAL_STRING("percent", ctx.last_record->kvs[0].key);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_KV_UINT, (uint32_t)ctx.last_record->kvs[0].type);
    TEST_ASSERT_EQUAL_UINT32(42, ctx.last_record->kvs[0].value.u32);
    TEST_ASSERT_EQUAL_STRING("menu", ctx.last_record->kvs[1].key);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_KV_BOOL, (uint32_t)ctx.last_record->kvs[1].type);
    TEST_ASSERT_TRUE(ctx.last_record->kvs[1].value.b);
}

TEST_CASE("log sink null and missing-op safety", "[blinky_log]")
{
    fake_log_ctx_t ctx = {0};
    const blinky_log_sink_ops_t no_emit_ops = {
        .emit = NULL,
    };
    blinky_log_sink_t sink_no_ops = {
        .ops = NULL,
        .ctx = &ctx,
    };
    blinky_log_sink_t sink_no_emit = {
        .ops = &no_emit_ops,
        .ctx = &ctx,
    };
    const blinky_log_record_t record = {
        .level = BLINKY_LOG_LEVEL_DEBUG,
        .domain = "test",
        .event = "noop",
        .message = "noop",
        .kvs = NULL,
        .kv_count = 0,
    };

    blinky_log_emit(NULL, &record);
    blinky_log_emit(&sink_no_ops, &record);
    blinky_log_emit(&sink_no_emit, &record);

    TEST_ASSERT_EQUAL_UINT32(0, ctx.calls);
}
