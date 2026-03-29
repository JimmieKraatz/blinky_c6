#include "unity.h"

#include "app_settings_store_idf.h"

TEST_CASE("idf settings store saves and loads boot pattern with placeholder settings", "[app_settings_store_idf]")
{
    app_settings_store_t store = {0};
    app_settings_store_idf_t idf = {0};
    app_settings_t saved = {0};
    app_settings_t loaded = {0};
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = true,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };

    app_settings_defaults(&saved, &defaults);
    saved.test_counter = 7U;
    saved.test_mode_enabled = true;
    saved.boot_pattern_enabled = false;
    saved.log_intensity_enabled = true;
    saved.log_min_level = BLINKY_LOG_LEVEL_DEBUG;
    saved.startup_selector = LED_STARTUP_SELECT_TRIANGLE;

    TEST_ASSERT_EQUAL(ESP_OK,
                      app_settings_store_idf_init(&store,
                                                  &idf,
                                                  &(app_settings_store_idf_config_t){
                                                      .partition_label = "nvs",
                                                      .namespace_name = "apptest",
                                                  }));
    TEST_ASSERT_NOT_NULL(store.ops);
    TEST_ASSERT_EQUAL(APP_SETTINGS_STORE_OK, store.ops->reset(store.ctx));
    TEST_ASSERT_EQUAL(APP_SETTINGS_STORE_OK, store.ops->save(store.ctx, &saved));
    TEST_ASSERT_EQUAL(APP_SETTINGS_STORE_OK, store.ops->load(store.ctx, &loaded));
    TEST_ASSERT_EQUAL_UINT32(saved.schema_version, loaded.schema_version);
    TEST_ASSERT_EQUAL(saved.boot_pattern_enabled, loaded.boot_pattern_enabled);
    TEST_ASSERT_EQUAL(saved.log_intensity_enabled, loaded.log_intensity_enabled);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)saved.log_min_level, (uint32_t)loaded.log_min_level);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)saved.startup_selector, (uint32_t)loaded.startup_selector);
    TEST_ASSERT_EQUAL_UINT32(saved.test_counter, loaded.test_counter);
    TEST_ASSERT_EQUAL(saved.test_mode_enabled, loaded.test_mode_enabled);
}

TEST_CASE("idf settings store reset clears namespace data", "[app_settings_store_idf]")
{
    app_settings_store_t store = {0};
    app_settings_store_idf_t idf = {0};
    app_settings_t cfg = {0};
    app_settings_defaults(&cfg, NULL);
    cfg.test_counter = 11U;

    TEST_ASSERT_EQUAL(ESP_OK,
                      app_settings_store_idf_init(&store,
                                                  &idf,
                                                  &(app_settings_store_idf_config_t){
                                                      .partition_label = "nvs",
                                                      .namespace_name = "apptest",
                                                  }));
    TEST_ASSERT_EQUAL(APP_SETTINGS_STORE_OK, store.ops->save(store.ctx, &cfg));
    TEST_ASSERT_EQUAL(APP_SETTINGS_STORE_OK, store.ops->reset(store.ctx));
    TEST_ASSERT_EQUAL(APP_SETTINGS_STORE_ERR_NOT_FOUND, store.ops->load(store.ctx, &cfg));
}
