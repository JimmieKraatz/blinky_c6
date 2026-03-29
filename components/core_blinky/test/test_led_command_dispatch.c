#include "unity.h"

#include "led_command_dispatch.h"

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

static void init_runtime(led_runtime_t *rt, led_runtime_output_t *out, led_wave_t wave)
{
    led_model_config_t cfg = test_cfg();
    led_runtime_init(rt, &cfg, wave, 0, out);
}

TEST_CASE("command dispatch applies run from paused", "[led_command_dispatch]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};

    init_runtime(&rt, &out, LED_WAVE_SQUARE);
    led_runtime_step(&rt, 10, BLINKY_EVENT_SHORT_PRESS, &out);
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_RUN, 20, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, rt.state);
}

TEST_CASE("command dispatch applies toggle from running and paused", "[led_command_dispatch]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};

    init_runtime(&rt, &out, LED_WAVE_SQUARE);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_RUN_PAUSE_TOGGLE, 10, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_RUN_PAUSE_TOGGLE, 20, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, rt.state);
}

TEST_CASE("command dispatch applies menu enter from running and paused", "[led_command_dispatch]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};

    init_runtime(&rt, &out, LED_WAVE_SQUARE);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_MENU_ENTER, 10, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, rt.state);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_MENU_EXIT, 20, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, rt.state);

    led_runtime_step(&rt, 30, BLINKY_EVENT_SHORT_PRESS, &out);
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_MENU_ENTER, 40, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, rt.state);
}

TEST_CASE("command dispatch applies menu next and menu exit in menu", "[led_command_dispatch]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};

    init_runtime(&rt, &out, LED_WAVE_SQUARE);
    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_MENU_ENTER, 10, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, rt.state);
    TEST_ASSERT_EQUAL(LED_WAVE_SQUARE, rt.model.wave);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_MENU_NEXT, 20, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, rt.state);
    TEST_ASSERT_EQUAL(LED_WAVE_SAW_UP, rt.model.wave);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_APPLIED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_MENU_EXIT, 30, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, rt.state);
}

TEST_CASE("command dispatch ignores command invalid for current state", "[led_command_dispatch]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    fake_log_ctx_t log_ctx = {0};
    const blinky_log_sink_ops_t ops = {
        .emit = fake_emit,
    };
    blinky_log_sink_t sink = {
        .ops = &ops,
        .ctx = &log_ctx,
    };

    init_runtime(&rt, &out, LED_WAVE_SQUARE);
    led_runtime_set_log_sink(&rt, &sink);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_IGNORED,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_MENU_EXIT, 10, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, rt.state);
    TEST_ASSERT_FALSE(out.write_level);
    TEST_ASSERT_FALSE(out.write_brightness);
    TEST_ASSERT_GREATER_THAN_UINT32(0, log_ctx.calls);
    TEST_ASSERT_NOT_NULL(log_ctx.last_record);
    TEST_ASSERT_EQUAL_STRING("runtime", log_ctx.last_record->domain);
    TEST_ASSERT_EQUAL_STRING("command", log_ctx.last_record->event);
}

TEST_CASE("command dispatch reports none as invalid", "[led_command_dispatch]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};

    init_runtime(&rt, &out, LED_WAVE_SINE);

    TEST_ASSERT_EQUAL(LED_COMMAND_RESULT_INVALID,
                      led_command_dispatch(&rt, BLINKY_CONTROL_CMD_NONE, 10, &out));
    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, rt.state);
    TEST_ASSERT_FALSE(out.write_level);
    TEST_ASSERT_FALSE(out.write_brightness);
}
