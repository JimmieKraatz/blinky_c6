#include "unity.h"

#include "led_model.h"

static const led_model_config_t TEST_CFG = {
    .wave_period_ms = 1000,
    .poll_ms = 10,
    .sine_steps_max = 256,
    .saw_step_pct = 5,
};

/* Square wave should alternate between 0% and 100%. */
TEST_CASE("square toggles 0 and 100", "[led_model]")
{
    led_model_t m = {0};
    led_percent_t pct = 0;

    led_model_init(&m, &TEST_CFG);
    led_model_set_wave(&m, LED_WAVE_SQUARE, 0);

    TEST_ASSERT_TRUE(led_model_tick(&m, m.next_update, &pct));
    TEST_ASSERT_EQUAL_UINT8(100, pct);

    TEST_ASSERT_TRUE(led_model_tick(&m, m.next_update, &pct));
    TEST_ASSERT_EQUAL_UINT8(0, pct);
}

/* Saw up should wrap to 0% after reaching 100%. */
TEST_CASE("saw up wraps after 100", "[led_model]")
{
    led_model_t m = {0};
    led_percent_t pct = 0;

    led_model_init(&m, &TEST_CFG);
    led_model_set_wave(&m, LED_WAVE_SAW_UP, 0);

    for (int i = 0; i < 20; ++i) {
        TEST_ASSERT_TRUE(led_model_tick(&m, m.next_update, &pct));
    }
    TEST_ASSERT_EQUAL_UINT8(100, pct);

    TEST_ASSERT_TRUE(led_model_tick(&m, m.next_update, &pct));
    TEST_ASSERT_EQUAL_UINT8(0, pct);
}

/* Triangle should hit both endpoints before reversing. */
TEST_CASE("triangle reaches endpoints", "[led_model]")
{
    led_model_t m = {0};
    led_percent_t pct = 0;

    led_model_init(&m, &TEST_CFG);
    led_model_set_wave(&m, LED_WAVE_TRIANGLE, 0);

    for (int i = 0; i < 20; ++i) {
        TEST_ASSERT_TRUE(led_model_tick(&m, m.next_update, &pct));
    }
    TEST_ASSERT_EQUAL_UINT8(100, pct);

    for (int i = 0; i < 20; ++i) {
        TEST_ASSERT_TRUE(led_model_tick(&m, m.next_update, &pct));
    }
    TEST_ASSERT_EQUAL_UINT8(0, pct);
}

/* Sine LUT should stay within [0,100]% bounds. */
TEST_CASE("sine stays in range", "[led_model]")
{
    led_model_t m = {0};
    led_percent_t pct = 0;

    led_model_init(&m, &TEST_CFG);
    led_model_set_wave(&m, LED_WAVE_SINE, 0);

    for (int i = 0; i < ((int)led_model_sine_steps(&m) + 16); ++i) {
        TEST_ASSERT_TRUE(led_model_tick(&m, m.next_update, &pct));
        TEST_ASSERT_LESS_OR_EQUAL_UINT8(100, pct);
    }
}

/* Percent <-> raw conversions should round-trip within tolerance. */
TEST_CASE("percent raw conversion round trip", "[led_model]")
{
    for (uint8_t pct = 0; pct <= 100; pct += 5) {
        led_brightness_t raw = led_brightness_from_percent(pct);
        uint8_t out = led_percent_from_brightness(raw);
        TEST_ASSERT_INT_WITHIN(1, pct, out);
    }
}
