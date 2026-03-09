#include "unity.h"

#include "menu_logic.h"

/* Short press should advance the wave selection. */
TEST_CASE("menu short press cycles wave", "[menu]")
{
    led_wave_t wave = LED_WAVE_SQUARE;
    led_menu_action_t action = led_menu_handle_event(&wave, BLINKY_EVENT_SHORT_PRESS);

    TEST_ASSERT_EQUAL(LED_MENU_ACTION_WAVE_CHANGED, action);
    TEST_ASSERT_EQUAL(LED_WAVE_SAW_UP, wave);
}

/* Short press should wrap from last wave to first. */
TEST_CASE("menu short press wraps to start", "[menu]")
{
    led_wave_t wave = (led_wave_t)(LED_WAVE_COUNT - 1);
    led_menu_action_t action = led_menu_handle_event(&wave, BLINKY_EVENT_SHORT_PRESS);

    TEST_ASSERT_EQUAL(LED_MENU_ACTION_WAVE_CHANGED, action);
    TEST_ASSERT_EQUAL(LED_WAVE_SQUARE, wave);
}

/* Long press should exit the menu without changing wave. */
TEST_CASE("menu long press exits", "[menu]")
{
    led_wave_t wave = LED_WAVE_TRIANGLE;
    led_menu_action_t action = led_menu_handle_event(&wave, BLINKY_EVENT_LONG_PRESS);

    TEST_ASSERT_EQUAL(LED_MENU_ACTION_EXIT, action);
    TEST_ASSERT_EQUAL(LED_WAVE_TRIANGLE, wave);
}

/* No-event input should be ignored. */
TEST_CASE("menu ignores no-event", "[menu]")
{
    led_wave_t wave = LED_WAVE_SINE;
    led_menu_action_t action = led_menu_handle_event(&wave, BLINKY_EVENT_NONE);

    TEST_ASSERT_EQUAL(LED_MENU_ACTION_NONE, action);
    TEST_ASSERT_EQUAL(LED_WAVE_SINE, wave);
}
