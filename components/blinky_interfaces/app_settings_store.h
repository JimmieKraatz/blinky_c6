#pragma once

#include "app_settings.h"

typedef enum {
    APP_SETTINGS_STORE_OK = 0,
    APP_SETTINGS_STORE_ERR_INVALID_ARG,
    APP_SETTINGS_STORE_ERR_NOT_FOUND,
    APP_SETTINGS_STORE_ERR_INVALID_DATA,
    APP_SETTINGS_STORE_ERR_IO,
} app_settings_store_status_t;

typedef app_settings_store_status_t (*app_settings_load_fn)(void *ctx, app_settings_t *out);
typedef app_settings_store_status_t (*app_settings_save_fn)(void *ctx, const app_settings_t *cfg);
typedef app_settings_store_status_t (*app_settings_reset_fn)(void *ctx);

typedef struct {
    app_settings_load_fn load;
    app_settings_save_fn save;
    app_settings_reset_fn reset;
} app_settings_store_ops_t;

typedef struct {
    const app_settings_store_ops_t *ops;
    void *ctx;
} app_settings_store_t;
