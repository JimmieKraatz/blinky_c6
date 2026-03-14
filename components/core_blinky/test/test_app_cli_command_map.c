#include "unity.h"

#include "app_cli_command_map.h"

TEST_CASE("cli command map routes menu enter and exit to button long", "[app_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_LONG,
                      app_cli_command_map_to_app_event(BLINKY_CLI_CMD_MENU_ENTER));
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_LONG,
                      app_cli_command_map_to_app_event(BLINKY_CLI_CMD_MENU_EXIT));
}

TEST_CASE("cli command map routes menu next to button short", "[app_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT,
                      app_cli_command_map_to_app_event(BLINKY_CLI_CMD_MENU_NEXT));
}

TEST_CASE("cli command map routes run pause toggle to button short", "[app_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT,
                      app_cli_command_map_to_app_event(BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE));
}

TEST_CASE("cli command map routes run and pause to button short", "[app_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, app_cli_command_map_to_app_event(BLINKY_CLI_CMD_RUN));
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT, app_cli_command_map_to_app_event(BLINKY_CLI_CMD_PAUSE));
}

TEST_CASE("cli command map treats help and status as non-dispatch commands", "[app_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_NONE, app_cli_command_map_to_app_event(BLINKY_CLI_CMD_HELP));
    TEST_ASSERT_EQUAL(APP_EVENT_NONE, app_cli_command_map_to_app_event(BLINKY_CLI_CMD_STATUS));
    TEST_ASSERT_FALSE(app_cli_command_map_is_dispatchable(BLINKY_CLI_CMD_HELP));
    TEST_ASSERT_FALSE(app_cli_command_map_is_dispatchable(BLINKY_CLI_CMD_STATUS));
}

TEST_CASE("cli command map treats unknown enum values as non-dispatch", "[app_cli_command_map]")
{
    const blinky_cli_command_t unknown = (blinky_cli_command_t)999;
    TEST_ASSERT_EQUAL(APP_EVENT_NONE, app_cli_command_map_to_app_event(unknown));
    TEST_ASSERT_FALSE(app_cli_command_map_is_dispatchable(unknown));
}

TEST_CASE("cli command map gates explicit commands by runtime state", "[app_cli_command_map]")
{
    TEST_ASSERT_TRUE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_ENTER, LED_POLICY_RUNNING));
    TEST_ASSERT_TRUE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_ENTER, LED_POLICY_PAUSED));
    TEST_ASSERT_FALSE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_ENTER, LED_POLICY_MENU));

    TEST_ASSERT_FALSE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_EXIT, LED_POLICY_RUNNING));
    TEST_ASSERT_FALSE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_EXIT, LED_POLICY_PAUSED));
    TEST_ASSERT_TRUE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_EXIT, LED_POLICY_MENU));

    TEST_ASSERT_FALSE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_NEXT, LED_POLICY_RUNNING));
    TEST_ASSERT_TRUE(
        app_cli_command_map_is_allowed_in_state(BLINKY_CLI_CMD_MENU_NEXT, LED_POLICY_MENU));
}
