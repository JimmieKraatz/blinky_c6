#include "unity.h"

#include "led_policy.h"

TEST_CASE("running short press transitions to paused", "[led_policy]")
{
    led_policy_ctx_t c = {
        .menu_return_state = LED_POLICY_RUNNING,
        .menu_wave = LED_WAVE_SQUARE,
    };

    led_policy_step_result_t out = led_policy_step(&c, LED_POLICY_RUNNING, LED_WAVE_SINE, BLINKY_EVENT_SHORT_PRESS);
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, out.next_state);
    TEST_ASSERT_FALSE(out.menu_wave_changed);
}

TEST_CASE("running long press enters menu and snapshots current wave", "[led_policy]")
{
    led_policy_ctx_t c = {0};

    led_policy_step_result_t out = led_policy_step(&c, LED_POLICY_RUNNING, LED_WAVE_TRIANGLE, BLINKY_EVENT_LONG_PRESS);
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, out.next_state);
    TEST_ASSERT_EQUAL(LED_POLICY_RUNNING, c.menu_return_state);
    TEST_ASSERT_EQUAL(LED_WAVE_TRIANGLE, c.menu_wave);
}

TEST_CASE("paused long press enters menu and remembers paused return", "[led_policy]")
{
    led_policy_ctx_t c = {0};

    led_policy_step_result_t out = led_policy_step(&c, LED_POLICY_PAUSED, LED_WAVE_SAW_UP, BLINKY_EVENT_LONG_PRESS);
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, out.next_state);
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, c.menu_return_state);
    TEST_ASSERT_EQUAL(LED_WAVE_SAW_UP, c.menu_wave);
}

TEST_CASE("menu short press changes menu wave", "[led_policy]")
{
    led_policy_ctx_t c = {
        .menu_return_state = LED_POLICY_RUNNING,
        .menu_wave = LED_WAVE_SQUARE,
    };

    led_policy_step_result_t out = led_policy_step(&c, LED_POLICY_MENU, LED_WAVE_SQUARE, BLINKY_EVENT_SHORT_PRESS);
    TEST_ASSERT_EQUAL(LED_POLICY_MENU, out.next_state);
    TEST_ASSERT_TRUE(out.menu_wave_changed);
    TEST_ASSERT_EQUAL(LED_WAVE_SAW_UP, c.menu_wave);
}

TEST_CASE("menu long press exits to remembered return state", "[led_policy]")
{
    led_policy_ctx_t c = {
        .menu_return_state = LED_POLICY_PAUSED,
        .menu_wave = LED_WAVE_SINE,
    };

    led_policy_step_result_t out = led_policy_step(&c, LED_POLICY_MENU, LED_WAVE_SINE, BLINKY_EVENT_LONG_PRESS);
    TEST_ASSERT_EQUAL(LED_POLICY_PAUSED, out.next_state);
    TEST_ASSERT_FALSE(out.menu_wave_changed);
}
