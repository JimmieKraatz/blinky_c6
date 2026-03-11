#include "unity.h"

#include "app_event.h"
#include "led_event_consumer.h"

static led_model_config_t test_cfg(void)
{
    return (led_model_config_t){
        .wave_period_ms = 1000,
        .poll_ms = 10,
        .sine_steps_max = 64,
        .saw_step_pct = 5,
    };
}

TEST_CASE("consumer maps short press to paused transition", "[led_event_consumer]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();

    led_runtime_init(&rt, &cfg, LED_WAVE_SQUARE, 0, &out);
    led_event_consumer_dispatch(&rt,
                                &(app_event_t){.type = APP_EVENT_BUTTON_SHORT, .timestamp_ms = 10},
                                &out);

    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, rt.state);
    TEST_ASSERT_TRUE(out.write_level);
}

TEST_CASE("consumer ignores unsupported event types", "[led_event_consumer]")
{
    led_runtime_t rt = {0};
    led_runtime_output_t out = {0};
    led_model_config_t cfg = test_cfg();
    led_policy_state_t before = LED_POLICY_RUNNING;

    led_runtime_init(&rt, &cfg, LED_WAVE_SINE, 0, &out);
    before = rt.state;

    led_event_consumer_dispatch(&rt,
                                &(app_event_t){.type = APP_EVENT_FAULT, .timestamp_ms = 20},
                                &out);

    TEST_ASSERT_EQUAL(before, rt.state);
    TEST_ASSERT_FALSE(out.write_level);
    TEST_ASSERT_FALSE(out.write_brightness);
}
