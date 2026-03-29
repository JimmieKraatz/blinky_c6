#include "unity.h"

#include "led_runtime.h"

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

static led_model_config_t test_cfg(void)
{
    return (led_model_config_t){
        .wave_period_ms = 1000,
        .poll_ms = 10,
        .sine_steps_max = 64,
        .saw_step_pct = 5,
    };
}

TEST_CASE("runtime init enters running and drives LED off", "[led_runtime]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();

    led_runtime_init(&rt, &cfg, LED_WAVE_SINE, 0, &out);

    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, rt.state);
    TEST_ASSERT_TRUE(out.write_level);
    TEST_ASSERT_FALSE(out.level_on);
}

TEST_CASE("runtime short press toggles running to paused", "[led_runtime]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();

    led_runtime_init(&rt, &cfg, LED_WAVE_SQUARE, 0, &out);
    led_runtime_step(&rt, 10, BLINKY_EVENT_SHORT_PRESS, &out);

    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);
    TEST_ASSERT_TRUE(out.write_level);
    TEST_ASSERT_FALSE(out.level_on);
}

TEST_CASE("runtime pause suppresses same-step brightness update", "[led_runtime]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();

    led_runtime_init(&rt, &cfg, LED_WAVE_SINE, 0, &out);
    led_runtime_step(&rt, 16, BLINKY_EVENT_SHORT_PRESS, &out);

    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);
    TEST_ASSERT_TRUE(out.write_level);
    TEST_ASSERT_FALSE(out.level_on);
    TEST_ASSERT_FALSE(out.write_brightness);
}

TEST_CASE("runtime menu returns to previous paused state", "[led_runtime]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();

    led_runtime_init(&rt, &cfg, LED_WAVE_SAW_UP, 0, &out);
    led_runtime_step(&rt, 10, BLINKY_EVENT_SHORT_PRESS, &out); /* paused */
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);

    led_runtime_step(&rt, 20, BLINKY_EVENT_LONG_PRESS, &out); /* enter menu */
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, rt.state);

    led_runtime_step(&rt, 30, BLINKY_EVENT_LONG_PRESS, &out); /* exit menu */
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);
}

TEST_CASE("runtime menu short press changes selected wave", "[led_runtime]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();

    led_runtime_init(&rt, &cfg, LED_WAVE_SQUARE, 0, &out);
    led_runtime_step(&rt, 10, BLINKY_EVENT_LONG_PRESS, &out); /* enter menu */
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, rt.state);
    TEST_ASSERT_EQUAL(LED_WAVE_SQUARE, rt.model.wave);

    led_runtime_step(&rt, 20, BLINKY_EVENT_SHORT_PRESS, &out); /* cycle wave */
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, rt.state);
    TEST_ASSERT_EQUAL(LED_WAVE_SAW_UP, rt.model.wave);
}

TEST_CASE("runtime emits structured logs when sink is configured", "[led_runtime]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();
    fake_log_ctx_t log_ctx = {0};
    const blinky_log_sink_ops_t ops = {
        .emit = fake_emit,
    };
    blinky_log_sink_t sink = {
        .ops = &ops,
        .ctx = &log_ctx,
    };

    led_runtime_init(&rt, &cfg, LED_WAVE_SQUARE, 0, &out);
    led_runtime_set_log_sink(&rt, &sink);
    led_runtime_step(&rt, 10, BLINKY_EVENT_LONG_PRESS, &out);  /* enter menu */
    led_runtime_step(&rt, 20, BLINKY_EVENT_SHORT_PRESS, &out); /* menu wave changed */

    TEST_ASSERT_GREATER_THAN_UINT32(0, log_ctx.calls);
    TEST_ASSERT_NOT_NULL(log_ctx.last_record);
    TEST_ASSERT_EQUAL_STRING("runtime", log_ctx.last_record->domain);
}

TEST_CASE("runtime init preserves preconfigured sink and emits init log", "[led_runtime]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();
    fake_log_ctx_t log_ctx = {0};
    const blinky_log_sink_ops_t ops = {
        .emit = fake_emit,
    };
    blinky_log_sink_t sink = {
        .ops = &ops,
        .ctx = &log_ctx,
    };

    led_runtime_set_log_sink(&rt, &sink);
    led_runtime_init(&rt, &cfg, LED_WAVE_SINE, 0, &out);

    TEST_ASSERT_GREATER_THAN_UINT32(0, log_ctx.calls);
    TEST_ASSERT_NOT_NULL(log_ctx.last_record);
    TEST_ASSERT_EQUAL_STRING("runtime", log_ctx.last_record->domain);
    TEST_ASSERT_EQUAL_STRING("state_change", log_ctx.last_record->event);
    TEST_ASSERT_EQUAL_STRING("running", log_ctx.last_record->message);
}
