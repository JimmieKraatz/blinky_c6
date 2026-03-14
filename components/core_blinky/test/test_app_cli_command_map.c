#include "unity.h"

#include "app_cli_command_map.h"

TEST_CASE("cli command map routes menu enter and exit to explicit blinky commands",
          "[app_cli_command_map]")
{
    blinky_control_command_t cmd = BLINKY_CONTROL_CMD_NONE;
    TEST_ASSERT_TRUE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_MENU_ENTER, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_MENU_ENTER, cmd);
    TEST_ASSERT_TRUE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_MENU_EXIT, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_MENU_EXIT, cmd);
}

TEST_CASE("cli command map routes menu next to explicit blinky command", "[app_cli_command_map]")
{
    blinky_control_command_t cmd = BLINKY_CONTROL_CMD_NONE;
    TEST_ASSERT_TRUE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_MENU_NEXT, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_MENU_NEXT, cmd);
}

TEST_CASE("cli command map routes run pause toggle to explicit blinky command", "[app_cli_command_map]")
{
    blinky_control_command_t cmd = BLINKY_CONTROL_CMD_NONE;
    TEST_ASSERT_TRUE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_RUN_PAUSE_TOGGLE, cmd);
}

TEST_CASE("cli command map routes run and pause to explicit blinky commands", "[app_cli_command_map]")
{
    blinky_control_command_t cmd = BLINKY_CONTROL_CMD_NONE;
    TEST_ASSERT_TRUE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_RUN, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_RUN, cmd);
    TEST_ASSERT_TRUE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_PAUSE, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_PAUSE, cmd);
}

TEST_CASE("cli command map treats help and status as non-dispatch commands", "[app_cli_command_map]")
{
    blinky_control_command_t cmd = BLINKY_CONTROL_CMD_MENU_ENTER;
    TEST_ASSERT_FALSE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_HELP, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_NONE, cmd);
    TEST_ASSERT_FALSE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_STATUS, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_NONE, cmd);
    TEST_ASSERT_FALSE(app_cli_command_map_is_dispatchable(BLINKY_CLI_CMD_HELP));
    TEST_ASSERT_FALSE(app_cli_command_map_is_dispatchable(BLINKY_CLI_CMD_STATUS));
}

TEST_CASE("cli command map treats unknown enum values as non-dispatch", "[app_cli_command_map]")
{
    const blinky_cli_command_t unknown = (blinky_cli_command_t)999;
    blinky_control_command_t cmd = BLINKY_CONTROL_CMD_MENU_ENTER;
    TEST_ASSERT_FALSE(app_cli_command_map_to_blinky_command(unknown, &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONTROL_CMD_NONE, cmd);
    TEST_ASSERT_FALSE(app_cli_command_map_is_dispatchable(unknown));
}

TEST_CASE("cli command map null output is non-dispatch", "[app_cli_command_map]")
{
    TEST_ASSERT_FALSE(app_cli_command_map_to_blinky_command(BLINKY_CLI_CMD_RUN, NULL));
}
