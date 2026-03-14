#include "app_settings.h"

void app_settings_defaults(app_settings_t *cfg)
{
    if (!cfg) {
        return;
    }

    *cfg = (app_settings_t){
        .schema_version = APP_SETTINGS_SCHEMA_VERSION,
        .test_counter = 0U,
        .test_mode_enabled = false,
    };
}

bool app_settings_is_valid(const app_settings_t *cfg)
{
    if (!cfg) {
        return false;
    }

    return cfg->schema_version == APP_SETTINGS_SCHEMA_VERSION;
}
