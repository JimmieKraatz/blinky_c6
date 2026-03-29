#pragma once

#include "esp_err.h"

#include "app_settings_store.h"

typedef struct {
    const char *partition_label;
    const char *namespace_name;
} app_settings_store_idf_config_t;

typedef struct {
    const char *partition_label;
    const char *namespace_name;
    bool partition_ready;
} app_settings_store_idf_t;

esp_err_t app_settings_store_idf_init(app_settings_store_t *store,
                                      app_settings_store_idf_t *idf,
                                      const app_settings_store_idf_config_t *cfg);
