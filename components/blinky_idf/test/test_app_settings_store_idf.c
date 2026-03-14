#include "unity.h"

#include "app_settings_store_idf.h"

TEST_CASE("idf settings store saves and loads placeholder settings", "[app_settings_store_idf]")
{
    app_settings_store_t store = {0};
    app_settings_store_idf_t idf = {0};
    app_settings_t saved = {0};
    app_settings_t loaded = {0};

    app_settings_defaults(&saved);
    saved.test_counter = 7U;
    saved.test_mode_enabled = true;

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
    TEST_ASSERT_EQUAL_UINT32(saved.test_counter, loaded.test_counter);
    TEST_ASSERT_EQUAL(saved.test_mode_enabled, loaded.test_mode_enabled);
}

TEST_CASE("idf settings store reset clears namespace data", "[app_settings_store_idf]")
{
    app_settings_store_t store = {0};
    app_settings_store_idf_t idf = {0};
    app_settings_t cfg = {0};
    app_settings_defaults(&cfg);
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
