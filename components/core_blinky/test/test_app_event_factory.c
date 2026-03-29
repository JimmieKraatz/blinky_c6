#include "unity.h"

#include "app_event_factory.h"
#include "blinky_control_command.h"

TEST_CASE("event factory boot builds app boot event with timestamp", "[app_event_factory]")
{
    app_event_t ev = app_event_factory_boot(321);
    TEST_ASSERT_EQUAL(APP_EVENT_BOOT, ev.type);
    TEST_ASSERT_EQUAL_UINT32(321, ev.timestamp_ms);
    TEST_ASSERT_EQUAL_UINT32(0, ev.payload.u32);
}

TEST_CASE("event factory maps short press to app button short", "[app_event_factory]")
{
    app_event_t ev = app_event_factory_from_input(BLINKY_EVENT_SHORT_PRESS, 55);
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, ev.type);
    TEST_ASSERT_EQUAL_UINT32(55, ev.timestamp_ms);
}

TEST_CASE("event factory maps long press to app button long", "[app_event_factory]")
{
    app_event_t ev = app_event_factory_from_input(BLINKY_EVENT_LONG_PRESS, 89);
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_LONG, ev.type);
    TEST_ASSERT_EQUAL_UINT32(89, ev.timestamp_ms);
}

TEST_CASE("event factory maps none to app tick", "[app_event_factory]")
{
    app_event_t ev = app_event_factory_from_input(BLINKY_EVENT_NONE, 13);
    TEST_ASSERT_EQUAL(APP_EVENT_TICK, ev.type);
    TEST_ASSERT_EQUAL_UINT32(13, ev.timestamp_ms);
}

TEST_CASE("event factory maps cli menu next to blinky command event", "[app_event_factory]")
{
    app_event_t ev = app_event_factory_from_cli_command(BLINKY_CLI_CMD_MENU_NEXT, 21);
    TEST_ASSERT_EQUAL(APP_EVENT_BLINKY_COMMAND, ev.type);
    TEST_ASSERT_EQUAL_UINT32(21, ev.timestamp_ms);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_CONTROL_CMD_MENU_NEXT, ev.payload.u32);
}

TEST_CASE("event factory maps cli help to non-dispatch event none", "[app_event_factory]")
{
    app_event_t ev = app_event_factory_from_cli_command(BLINKY_CLI_CMD_HELP, 22);
    TEST_ASSERT_EQUAL(APP_EVENT_NONE, ev.type);
    TEST_ASSERT_EQUAL_UINT32(22, ev.timestamp_ms);
}

TEST_CASE("event factory builds blinky command event directly", "[app_event_factory]")
{
    app_event_t ev = app_event_factory_from_blinky_command(BLINKY_CONTROL_CMD_RUN, 34);
    TEST_ASSERT_EQUAL(APP_EVENT_BLINKY_COMMAND, ev.type);
    TEST_ASSERT_EQUAL_UINT32(34, ev.timestamp_ms);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_CONTROL_CMD_RUN, ev.payload.u32);
}
