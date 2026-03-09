#include "unity.h"

#include "led_runtime.h"

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
