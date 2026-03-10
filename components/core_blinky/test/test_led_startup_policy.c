#include "unity.h"

#include "led_startup_policy.h"

TEST_CASE("startup policy returns configured valid wave", "[led_startup_policy]")
{
    led_startup_config_t cfg = {.start_wave = LED_WAVE_TRIANGLE};
    TEST_ASSERT_EQUAL(LED_WAVE_TRIANGLE, led_startup_policy_select_wave(&cfg));
}

TEST_CASE("startup policy defaults to sine on null config", "[led_startup_policy]")
{
    TEST_ASSERT_EQUAL(LED_WAVE_SINE, led_startup_policy_select_wave(NULL));
}

TEST_CASE("startup policy defaults to sine on invalid wave", "[led_startup_policy]")
{
    led_startup_config_t cfg = {.start_wave = (led_wave_t)99};
    TEST_ASSERT_EQUAL(LED_WAVE_SINE, led_startup_policy_select_wave(&cfg));
}
