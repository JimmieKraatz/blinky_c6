#include "unity.h"

#include "app_settings.h"

TEST_CASE("settings defaults initialize persisted payload from provided defaults", "[app_settings]")
{
    app_settings_t cfg = {0};
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = true,
        .log_intensity_enabled = true,
        .log_min_level = BLINKY_LOG_LEVEL_DEBUG,
        .startup_selector = LED_STARTUP_SELECT_TRIANGLE,
    };

    app_settings_defaults(&cfg, &defaults);

    TEST_ASSERT_EQUAL_UINT32(APP_SETTINGS_SCHEMA_VERSION, cfg.schema_version);
    TEST_ASSERT_TRUE(cfg.boot_pattern_enabled);
    TEST_ASSERT_TRUE(cfg.log_intensity_enabled);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_DEBUG, (uint32_t)cfg.log_min_level);
    TEST_ASSERT_EQUAL_UINT32(LED_STARTUP_SELECT_TRIANGLE, (uint32_t)cfg.startup_selector);
    TEST_ASSERT_EQUAL_UINT32(0, cfg.test_counter);
    TEST_ASSERT_FALSE(cfg.test_mode_enabled);
}

TEST_CASE("settings defaults fall back to disabled boot pattern without defaults input", "[app_settings]")
{
    app_settings_t cfg = {0};

    app_settings_defaults(&cfg, NULL);

    TEST_ASSERT_EQUAL_UINT32(APP_SETTINGS_SCHEMA_VERSION, cfg.schema_version);
    TEST_ASSERT_FALSE(cfg.boot_pattern_enabled);
    TEST_ASSERT_FALSE(cfg.log_intensity_enabled);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_INFO, (uint32_t)cfg.log_min_level);
    TEST_ASSERT_EQUAL_UINT32(LED_STARTUP_SELECT_DEFAULT, (uint32_t)cfg.startup_selector);
}

TEST_CASE("settings validation accepts current schema version", "[app_settings]")
{
    app_settings_t cfg = {0};

    app_settings_defaults(&cfg, NULL);

    TEST_ASSERT_TRUE(app_settings_is_valid(&cfg));
}

TEST_CASE("settings validation rejects null and wrong schema version", "[app_settings]")
{
    app_settings_t cfg = {
        .schema_version = APP_SETTINGS_SCHEMA_VERSION + 1U,
        .boot_pattern_enabled = true,
        .log_intensity_enabled = true,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
        .test_counter = 5U,
        .test_mode_enabled = true,
    };

    TEST_ASSERT_FALSE(app_settings_is_valid(NULL));
    TEST_ASSERT_FALSE(app_settings_is_valid(&cfg));

    cfg.schema_version = APP_SETTINGS_SCHEMA_VERSION;
    cfg.log_min_level = (blinky_log_level_t)99;
    TEST_ASSERT_FALSE(app_settings_is_valid(&cfg));

    cfg.log_min_level = BLINKY_LOG_LEVEL_INFO;
    cfg.startup_selector = (led_startup_selector_t)99;
    TEST_ASSERT_FALSE(app_settings_is_valid(&cfg));
}
