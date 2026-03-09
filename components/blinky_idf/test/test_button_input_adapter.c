#include "unity.h"

#include "button_input_adapter.h"

typedef struct {
    int poll_calls;
    int now_calls;
} fake_input_ctx_t;

static blinky_event_t fake_poll_event(void *ctx)
{
    fake_input_ctx_t *f = (fake_input_ctx_t *)ctx;
    f->poll_calls++;
    return BLINKY_EVENT_LONG_PRESS;
}

static blinky_time_ms_t fake_now_ms(void *ctx)
{
    fake_input_ctx_t *f = (fake_input_ctx_t *)ctx;
    f->now_calls++;
    return 4242;
}

TEST_CASE("input adapter dispatches poll_event and now_ms", "[button_input_adapter]")
{
    fake_input_ctx_t fake = {0};
    button_input_adapter_t adapter = {
        .ops = &(button_input_adapter_ops_t){
            .poll_event = fake_poll_event,
            .now_ms = fake_now_ms,
        },
        .ctx = &fake,
    };

    TEST_ASSERT_EQUAL(BLINKY_EVENT_LONG_PRESS, button_input_adapter_poll_event(&adapter));
    TEST_ASSERT_EQUAL_UINT32(4242, button_input_adapter_now_ms(&adapter));
    TEST_ASSERT_EQUAL_INT(1, fake.poll_calls);
    TEST_ASSERT_EQUAL_INT(1, fake.now_calls);
}

TEST_CASE("input adapter null-safe defaults", "[button_input_adapter]")
{
    button_input_adapter_t adapter = {
        .ops = NULL,
        .ctx = NULL,
    };

    TEST_ASSERT_EQUAL(BLINKY_EVENT_NONE, button_input_adapter_poll_event(&adapter));
    TEST_ASSERT_EQUAL_UINT32(0, button_input_adapter_now_ms(&adapter));
    TEST_ASSERT_EQUAL(BLINKY_EVENT_NONE, button_input_adapter_poll_event(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, button_input_adapter_now_ms(NULL));
}

TEST_CASE("input adapter tolerates missing individual ops", "[button_input_adapter]")
{
    fake_input_ctx_t fake = {0};
    button_input_adapter_t adapter = {
        .ops = &(button_input_adapter_ops_t){
            .poll_event = fake_poll_event,
            .now_ms = NULL,
        },
        .ctx = &fake,
    };

    TEST_ASSERT_EQUAL(BLINKY_EVENT_LONG_PRESS, button_input_adapter_poll_event(&adapter));
    TEST_ASSERT_EQUAL_UINT32(0, button_input_adapter_now_ms(&adapter));
    TEST_ASSERT_EQUAL_INT(1, fake.poll_calls);
    TEST_ASSERT_EQUAL_INT(0, fake.now_calls);
}
