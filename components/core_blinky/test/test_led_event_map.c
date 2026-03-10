#include "unity.h"

#include "led_event_map.h"

TEST_CASE("event map converts blinky short press to app button short", "[led_event_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, led_event_map_from_blinky(BLINKY_EVENT_SHORT_PRESS));
}

TEST_CASE("event map converts blinky long press to app button long", "[led_event_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_LONG, led_event_map_from_blinky(BLINKY_EVENT_LONG_PRESS));
}

TEST_CASE("event map converts blinky none to app tick", "[led_event_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_TICK, led_event_map_from_blinky(BLINKY_EVENT_NONE));
}

TEST_CASE("event map converts app button short to blinky short press", "[led_event_map]")
{
    TEST_ASSERT_EQUAL(BLINKY_EVENT_SHORT_PRESS, led_event_map_to_blinky(APP_EVENT_BUTTON_SHORT));
}

TEST_CASE("event map converts app button long to blinky long press", "[led_event_map]")
{
    TEST_ASSERT_EQUAL(BLINKY_EVENT_LONG_PRESS, led_event_map_to_blinky(APP_EVENT_BUTTON_LONG));
}

TEST_CASE("event map converts app tick to blinky none", "[led_event_map]")
{
    TEST_ASSERT_EQUAL(BLINKY_EVENT_NONE, led_event_map_to_blinky(APP_EVENT_TICK));
}

TEST_CASE("event map dispatchable set covers current runtime events", "[led_event_map]")
{
    TEST_ASSERT_TRUE(led_event_map_is_dispatchable(APP_EVENT_BOOT));
    TEST_ASSERT_TRUE(led_event_map_is_dispatchable(APP_EVENT_TICK));
    TEST_ASSERT_TRUE(led_event_map_is_dispatchable(APP_EVENT_BUTTON_SHORT));
    TEST_ASSERT_TRUE(led_event_map_is_dispatchable(APP_EVENT_BUTTON_LONG));
    TEST_ASSERT_FALSE(led_event_map_is_dispatchable(APP_EVENT_FAULT));
}
