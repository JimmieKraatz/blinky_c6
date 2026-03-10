#include "unity.h"

#include "led_event_factory.h"

TEST_CASE("event factory boot builds app boot event with timestamp", "[led_event_factory]")
{
    app_event_t ev = led_event_factory_boot(321);
    TEST_ASSERT_EQUAL(APP_EVENT_BOOT, ev.type);
    TEST_ASSERT_EQUAL_UINT32(321, ev.timestamp_ms);
    TEST_ASSERT_EQUAL_UINT32(0, ev.payload.u32);
}

TEST_CASE("event factory maps short press to app button short", "[led_event_factory]")
{
    app_event_t ev = led_event_factory_from_input(BLINKY_EVENT_SHORT_PRESS, 55);
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, ev.type);
    TEST_ASSERT_EQUAL_UINT32(55, ev.timestamp_ms);
}

TEST_CASE("event factory maps long press to app button long", "[led_event_factory]")
{
    app_event_t ev = led_event_factory_from_input(BLINKY_EVENT_LONG_PRESS, 89);
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_LONG, ev.type);
    TEST_ASSERT_EQUAL_UINT32(89, ev.timestamp_ms);
}

TEST_CASE("event factory maps none to app tick", "[led_event_factory]")
{
    app_event_t ev = led_event_factory_from_input(BLINKY_EVENT_NONE, 13);
    TEST_ASSERT_EQUAL(APP_EVENT_TICK, ev.type);
    TEST_ASSERT_EQUAL_UINT32(13, ev.timestamp_ms);
}
