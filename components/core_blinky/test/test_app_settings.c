#include "unity.h"

#include "app_settings.h"

TEST_CASE("settings defaults initialize placeholder persisted payload", "[app_settings]")
{
    app_settings_t cfg = {0};

    app_settings_defaults(&cfg);

    TEST_ASSERT_EQUAL_UINT32(APP_SETTINGS_SCHEMA_VERSION, cfg.schema_version);
    TEST_ASSERT_EQUAL_UINT32(0, cfg.test_counter);
    TEST_ASSERT_FALSE(cfg.test_mode_enabled);
}

TEST_CASE("settings validation accepts current schema version", "[app_settings]")
{
    app_settings_t cfg = {0};

    app_settings_defaults(&cfg);

    TEST_ASSERT_TRUE(app_settings_is_valid(&cfg));
}

TEST_CASE("settings validation rejects null and wrong schema version", "[app_settings]")
{
    app_settings_t cfg = {
        .schema_version = APP_SETTINGS_SCHEMA_VERSION + 1U,
        .test_counter = 5U,
        .test_mode_enabled = true,
    };

    TEST_ASSERT_FALSE(app_settings_is_valid(NULL));
    TEST_ASSERT_FALSE(app_settings_is_valid(&cfg));
}
