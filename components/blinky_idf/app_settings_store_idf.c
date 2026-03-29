#include "app_settings_store_idf.h"

#include <stdbool.h>

#include "nvs.h"
#include "nvs_flash.h"

static app_settings_store_status_t status_from_esp_err(esp_err_t err)
{
    switch (err) {
    case ESP_OK:
        return APP_SETTINGS_STORE_OK;
    case ESP_ERR_INVALID_ARG:
        return APP_SETTINGS_STORE_ERR_INVALID_ARG;
    case ESP_ERR_NVS_NOT_FOUND:
        return APP_SETTINGS_STORE_ERR_NOT_FOUND;
    default:
        return APP_SETTINGS_STORE_ERR_IO;
    }
}

static esp_err_t ensure_partition_ready(app_settings_store_idf_t *idf)
{
    if (!idf || !idf->partition_label) {
        return ESP_ERR_INVALID_ARG;
    }
    if (idf->partition_ready) {
        return ESP_OK;
    }

    esp_err_t err = nvs_flash_init_partition(idf->partition_label);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err = nvs_flash_erase_partition(idf->partition_label);
        if (err != ESP_OK) {
            return err;
        }
        err = nvs_flash_init_partition(idf->partition_label);
    }
    if (err == ESP_OK) {
        idf->partition_ready = true;
    }
    return err;
}

static app_settings_store_status_t store_load(void *ctx, app_settings_t *out)
{
    app_settings_store_idf_t *idf = (app_settings_store_idf_t *)ctx;
    if (!out) {
        return APP_SETTINGS_STORE_ERR_INVALID_ARG;
    }

    esp_err_t err = ensure_partition_ready(idf);
    if (err != ESP_OK) {
        return status_from_esp_err(err);
    }

    nvs_handle_t handle = 0;
    err = nvs_open_from_partition(idf->partition_label, idf->namespace_name, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return status_from_esp_err(err);
    }

    app_settings_t cfg = {0};
    uint8_t mode = 0U;
    uint8_t boot_pattern = 0U;
    uint8_t log_intensity = 0U;
    uint8_t log_min_level = 0U;
    uint8_t startup_wave = 0U;
    err = nvs_get_u32(handle, "schema_ver", &cfg.schema_version);
    if (err == ESP_OK) {
        err = nvs_get_u8(handle, "boot_pattern", &boot_pattern);
    }
    if (err == ESP_OK) {
        err = nvs_get_u8(handle, "log_intensity", &log_intensity);
    }
    if (err == ESP_OK) {
        err = nvs_get_u8(handle, "log_level", &log_min_level);
    }
    if (err == ESP_OK) {
        err = nvs_get_u8(handle, "startup_wave", &startup_wave);
    }
    if (err == ESP_OK) {
        err = nvs_get_u32(handle, "test_count", &cfg.test_counter);
    }
    if (err == ESP_OK) {
        err = nvs_get_u8(handle, "test_mode", &mode);
    }
    nvs_close(handle);
    if (err != ESP_OK) {
        return status_from_esp_err(err);
    }

    cfg.boot_pattern_enabled = (boot_pattern != 0U);
    cfg.log_intensity_enabled = (log_intensity != 0U);
    cfg.log_min_level = (blinky_log_level_t)log_min_level;
    cfg.startup_selector = (led_startup_selector_t)startup_wave;
    cfg.test_mode_enabled = (mode != 0U);
    if (!app_settings_is_valid(&cfg)) {
        return APP_SETTINGS_STORE_ERR_INVALID_DATA;
    }

    *out = cfg;
    return APP_SETTINGS_STORE_OK;
}

static app_settings_store_status_t store_save(void *ctx, const app_settings_t *cfg)
{
    app_settings_store_idf_t *idf = (app_settings_store_idf_t *)ctx;
    if (!cfg) {
        return APP_SETTINGS_STORE_ERR_INVALID_ARG;
    }
    if (!app_settings_is_valid(cfg)) {
        return APP_SETTINGS_STORE_ERR_INVALID_DATA;
    }

    esp_err_t err = ensure_partition_ready(idf);
    if (err != ESP_OK) {
        return status_from_esp_err(err);
    }

    nvs_handle_t handle = 0;
    err = nvs_open_from_partition(idf->partition_label, idf->namespace_name, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return status_from_esp_err(err);
    }

    err = nvs_set_u32(handle, "schema_ver", cfg->schema_version);
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, "boot_pattern", cfg->boot_pattern_enabled ? 1U : 0U);
    }
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, "log_intensity", cfg->log_intensity_enabled ? 1U : 0U);
    }
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, "log_level", (uint8_t)cfg->log_min_level);
    }
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, "startup_wave", (uint8_t)cfg->startup_selector);
    }
    if (err == ESP_OK) {
        err = nvs_set_u32(handle, "test_count", cfg->test_counter);
    }
    if (err == ESP_OK) {
        err = nvs_set_u8(handle, "test_mode", cfg->test_mode_enabled ? 1U : 0U);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return status_from_esp_err(err);
}

static app_settings_store_status_t store_reset(void *ctx)
{
    app_settings_store_idf_t *idf = (app_settings_store_idf_t *)ctx;
    esp_err_t err = ensure_partition_ready(idf);
    if (err != ESP_OK) {
        return status_from_esp_err(err);
    }

    nvs_handle_t handle = 0;
    err = nvs_open_from_partition(idf->partition_label, idf->namespace_name, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return status_from_esp_err(err);
    }

    err = nvs_erase_all(handle);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return status_from_esp_err(err);
}

static const app_settings_store_ops_t STORE_OPS = {
    .load = store_load,
    .save = store_save,
    .reset = store_reset,
};

esp_err_t app_settings_store_idf_init(app_settings_store_t *store,
                                      app_settings_store_idf_t *idf,
                                      const app_settings_store_idf_config_t *cfg)
{
    if (!store || !idf || !cfg || !cfg->partition_label || !cfg->namespace_name) {
        return ESP_ERR_INVALID_ARG;
    }

    *idf = (app_settings_store_idf_t){
        .partition_label = cfg->partition_label,
        .namespace_name = cfg->namespace_name,
        .partition_ready = false,
    };
    *store = (app_settings_store_t){
        .ops = &STORE_OPS,
        .ctx = idf,
    };

    return ensure_partition_ready(idf);
}
