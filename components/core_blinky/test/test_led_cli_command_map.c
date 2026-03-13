#include "unity.h"

#include "led_cli_command_map.h"

TEST_CASE("cli command map routes menu enter and exit to button long", "[led_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_LONG,
                      led_cli_command_map_to_app_event(BLINKY_CLI_CMD_MENU_ENTER));
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_LONG,
                      led_cli_command_map_to_app_event(BLINKY_CLI_CMD_MENU_EXIT));
}

TEST_CASE("cli command map routes menu next to button short", "[led_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT,
                      led_cli_command_map_to_app_event(BLINKY_CLI_CMD_MENU_NEXT));
}

TEST_CASE("cli command map routes run pause toggle to button short", "[led_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_BUTTON_SHORT,
                      led_cli_command_map_to_app_event(BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE));
}

TEST_CASE("cli command map treats help and status as non-dispatch commands", "[led_cli_command_map]")
{
    TEST_ASSERT_EQUAL(APP_EVENT_NONE, led_cli_command_map_to_app_event(BLINKY_CLI_CMD_HELP));
    TEST_ASSERT_EQUAL(APP_EVENT_NONE, led_cli_command_map_to_app_event(BLINKY_CLI_CMD_STATUS));
    TEST_ASSERT_FALSE(led_cli_command_map_is_dispatchable(BLINKY_CLI_CMD_HELP));
    TEST_ASSERT_FALSE(led_cli_command_map_is_dispatchable(BLINKY_CLI_CMD_STATUS));
}

