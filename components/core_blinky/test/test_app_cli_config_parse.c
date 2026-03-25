#include "unity.h"

#include "app_cli_parse.h"

TEST_CASE("config parser maps show commands", "[app_cli_parse]")
{
    blinky_config_command_t cmd = {0};

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config show", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SHOW, cmd.action);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_VIEW_ALL, cmd.view);

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config show startup", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SHOW, cmd.action);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_VIEW_STARTUP, cmd.view);

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config show logging", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SHOW, cmd.action);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_VIEW_LOGGING, cmd.view);
}

TEST_CASE("config parser maps setter commands", "[app_cli_parse]")
{
    blinky_config_command_t cmd = {0};

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config startup wave triangle", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SET, cmd.action);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_KEY_STARTUP_WAVE, cmd.key);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_VALUE_STARTUP_WAVE, cmd.value.kind);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_STARTUP_WAVE_TRIANGLE, cmd.value.as.startup_wave);

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config boot-pattern on", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SET, cmd.action);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_KEY_BOOT_PATTERN, cmd.key);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_VALUE_BOOL, cmd.value.kind);
    TEST_ASSERT_TRUE(cmd.value.as.bool_value);

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config log intensity off", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SET, cmd.action);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_KEY_LOG_INTENSITY, cmd.key);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_VALUE_BOOL, cmd.value.kind);
    TEST_ASSERT_FALSE(cmd.value.as.bool_value);

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config log level debug", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SET, cmd.action);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_KEY_LOG_LEVEL, cmd.key);
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_VALUE_LOG_LEVEL, cmd.value.kind);
    TEST_ASSERT_EQUAL(BLINKY_LOG_LEVEL_DEBUG, cmd.value.as.log_level);
}

TEST_CASE("config parser maps save and reset commands", "[app_cli_parse]")
{
    blinky_config_command_t cmd = {0};

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config save", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_SAVE, cmd.action);

    TEST_ASSERT_TRUE(app_cli_parse_config_command("config reset", &cmd));
    TEST_ASSERT_EQUAL(BLINKY_CONFIG_CMD_RESET, cmd.action);
}

TEST_CASE("config parser rejects null and unknown input", "[app_cli_parse]")
{
    blinky_config_command_t cmd = {0};

    TEST_ASSERT_FALSE(app_cli_parse_config_command(NULL, &cmd));
    TEST_ASSERT_FALSE(app_cli_parse_config_command("", &cmd));
    TEST_ASSERT_FALSE(app_cli_parse_config_command("config", &cmd));
    TEST_ASSERT_FALSE(app_cli_parse_config_command("config startup mode paused", &cmd));
    TEST_ASSERT_FALSE(app_cli_parse_config_command("config log level trace", &cmd));
    TEST_ASSERT_FALSE(app_cli_parse_config_command("config boot-pattern maybe", &cmd));
    TEST_ASSERT_FALSE(app_cli_parse_config_command("status", &cmd));
}
