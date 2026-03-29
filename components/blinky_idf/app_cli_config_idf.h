#pragma once

#include <stdbool.h>

#include "app_settings.h"
#include "app_settings_store.h"
#include "blinky_config_command.h"
#include "blinky_log.h"

typedef void (*app_cli_config_report_fn)(void *ctx, blinky_log_level_t level, const char *line);
typedef void (*app_cli_config_apply_fn)(void *ctx, const app_settings_t *cfg);

typedef struct {
    app_settings_store_t *store;
    app_settings_defaults_t defaults;
    app_settings_t current;
    app_cli_config_report_fn report;
    void *report_ctx;
    app_cli_config_apply_fn apply;
    void *apply_ctx;
    bool dirty;
} app_cli_config_idf_t;

void app_cli_config_idf_init(app_cli_config_idf_t *cfg,
                             app_settings_store_t *store,
                             const app_settings_defaults_t *defaults,
                             const app_settings_t *initial,
                             app_cli_config_report_fn report,
                             void *report_ctx,
                             app_cli_config_apply_fn apply,
                             void *apply_ctx);

bool app_cli_config_idf_handle(app_cli_config_idf_t *cfg, const blinky_config_command_t *cmd);
