#include "app_settings.h"

static bool is_valid_log_level(blinky_log_level_t level)
{
    switch (level) {
    case BLINKY_LOG_LEVEL_ERROR:
    case BLINKY_LOG_LEVEL_WARN:
    case BLINKY_LOG_LEVEL_INFO:
    case BLINKY_LOG_LEVEL_DEBUG:
        return true;
    default:
        return false;
    }
}

static bool is_valid_startup_selector(led_startup_selector_t selector)
{
    switch (selector) {
    case LED_STARTUP_SELECT_DEFAULT:
    case LED_STARTUP_SELECT_SQUARE:
    case LED_STARTUP_SELECT_SAW_UP:
    case LED_STARTUP_SELECT_SAW_DOWN:
    case LED_STARTUP_SELECT_TRIANGLE:
    case LED_STARTUP_SELECT_SINE:
        return true;
    default:
        return false;
    }
}

void app_settings_defaults(app_settings_t *cfg, const app_settings_defaults_t *defaults)
{
    if (!cfg) {
        return;
    }

    *cfg = (app_settings_t){
        .schema_version = APP_SETTINGS_SCHEMA_VERSION,
        .boot_pattern_enabled = defaults ? defaults->boot_pattern_enabled : false,
        .log_intensity_enabled = defaults ? defaults->log_intensity_enabled : false,
        .log_min_level = defaults ? defaults->log_min_level : BLINKY_LOG_LEVEL_INFO,
        .startup_selector = defaults ? defaults->startup_selector : LED_STARTUP_SELECT_DEFAULT,
        .test_counter = 0U,
        .test_mode_enabled = false,
    };
}

bool app_settings_is_valid(const app_settings_t *cfg)
{
    if (!cfg) {
        return false;
    }

    return cfg->schema_version == APP_SETTINGS_SCHEMA_VERSION &&
           is_valid_log_level(cfg->log_min_level) &&
           is_valid_startup_selector(cfg->startup_selector);
}
