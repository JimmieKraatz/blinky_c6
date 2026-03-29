#include "unity.h"

#include "app_cli_parse.h"

TEST_CASE("cli parser maps primary commands", "[app_cli_parse]")
{
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_HELP, app_cli_parse_line("help"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_HELP_CONFIG, app_cli_parse_line("help config"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_STATUS, app_cli_parse_line("status"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_RUN, app_cli_parse_line("run"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_PAUSE, app_cli_parse_line("pause"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_MENU_ENTER, app_cli_parse_line("menu enter"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_MENU_NEXT, app_cli_parse_line("menu next"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_MENU_EXIT, app_cli_parse_line("menu exit"));
}

TEST_CASE("cli parser normalizes case and spacing", "[app_cli_parse]")
{
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_HELP, app_cli_parse_line("  HeLP "));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_HELP_CONFIG, app_cli_parse_line(" Help   Config "));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_MENU_NEXT, app_cli_parse_line(" MENU   NEXT "));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE, app_cli_parse_line("run   pause   toggle"));
}

TEST_CASE("cli parser returns none for empty and unknown", "[app_cli_parse]")
{
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_NONE, app_cli_parse_line(NULL));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_NONE, app_cli_parse_line(""));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_NONE, app_cli_parse_line("   "));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_NONE, app_cli_parse_line("menu prev"));
    TEST_ASSERT_EQUAL(BLINKY_CLI_CMD_NONE, app_cli_parse_line("foo"));
}
