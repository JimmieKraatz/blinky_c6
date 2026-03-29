#include "app_cli_config_idf.h"

#include <stdio.h>
#include <string.h>

static const char *on_off_name(bool value)
{
    return value ? "on" : "off";
}

static const char *log_level_name(blinky_log_level_t level)
{
    switch (level) {
    case BLINKY_LOG_LEVEL_ERROR:
        return "error";
    case BLINKY_LOG_LEVEL_WARN:
        return "warn";
    case BLINKY_LOG_LEVEL_DEBUG:
        return "debug";
    case BLINKY_LOG_LEVEL_INFO:
    default:
        return "info";
    }
}

static const char *startup_wave_name(led_startup_selector_t selector)
{
    switch (selector) {
    case LED_STARTUP_SELECT_SQUARE:
        return "square";
    case LED_STARTUP_SELECT_SAW_UP:
        return "saw_up";
    case LED_STARTUP_SELECT_SAW_DOWN:
        return "saw_down";
    case LED_STARTUP_SELECT_TRIANGLE:
        return "triangle";
    case LED_STARTUP_SELECT_SINE:
    case LED_STARTUP_SELECT_DEFAULT:
    default:
        return "sine";
    }
}

static led_startup_selector_t startup_selector_from_command(blinky_config_startup_wave_t wave)
{
    switch (wave) {
    case BLINKY_CONFIG_STARTUP_WAVE_SQUARE:
        return LED_STARTUP_SELECT_SQUARE;
    case BLINKY_CONFIG_STARTUP_WAVE_SAW_UP:
        return LED_STARTUP_SELECT_SAW_UP;
    case BLINKY_CONFIG_STARTUP_WAVE_SAW_DOWN:
        return LED_STARTUP_SELECT_SAW_DOWN;
    case BLINKY_CONFIG_STARTUP_WAVE_TRIANGLE:
        return LED_STARTUP_SELECT_TRIANGLE;
    case BLINKY_CONFIG_STARTUP_WAVE_SINE:
    case BLINKY_CONFIG_STARTUP_WAVE_NONE:
    default:
        return LED_STARTUP_SELECT_SINE;
    }
}

static void report_line(const app_cli_config_idf_t *cfg, blinky_log_level_t level, const char *line)
{
    if (!cfg || !cfg->report || !line) {
        return;
    }

    cfg->report(cfg->report_ctx, level, line);
}

static void apply_current(const app_cli_config_idf_t *cfg)
{
    if (!cfg || !cfg->apply) {
        return;
    }

    cfg->apply(cfg->apply_ctx, &cfg->current);
}

static void report_show_all(const app_cli_config_idf_t *cfg)
{
    char line[128] = {0};

    snprintf(line,
             sizeof(line),
             "config: startup_wave=%s boot_pattern=%s log_intensity=%s log_level=%s pending_save=%s",
             startup_wave_name(cfg->current.startup_selector),
             on_off_name(cfg->current.boot_pattern_enabled),
             on_off_name(cfg->current.log_intensity_enabled),
             log_level_name(cfg->current.log_min_level),
             on_off_name(cfg->dirty));
    report_line(cfg, BLINKY_LOG_LEVEL_INFO, line);
}

static void report_show_startup(const app_cli_config_idf_t *cfg)
{
    char line[128] = {0};

    snprintf(line,
             sizeof(line),
             "config startup: wave=%s boot_pattern=%s pending_save=%s",
             startup_wave_name(cfg->current.startup_selector),
             on_off_name(cfg->current.boot_pattern_enabled),
             on_off_name(cfg->dirty));
    report_line(cfg, BLINKY_LOG_LEVEL_INFO, line);
}

static void report_show_logging(const app_cli_config_idf_t *cfg)
{
    char line[128] = {0};

    snprintf(line,
             sizeof(line),
             "config logging: intensity=%s level=%s pending_save=%s",
             on_off_name(cfg->current.log_intensity_enabled),
             log_level_name(cfg->current.log_min_level),
             on_off_name(cfg->dirty));
    report_line(cfg, BLINKY_LOG_LEVEL_INFO, line);
}

static void report_set_result(const app_cli_config_idf_t *cfg, const char *key, const char *value)
{
    char line[128] = {0};

    snprintf(line,
             sizeof(line),
             "config updated: %s=%s (run 'config save' to persist)",
             key ? key : "unknown",
             value ? value : "unknown");
    report_line(cfg, BLINKY_LOG_LEVEL_INFO, line);
}

void app_cli_config_idf_init(app_cli_config_idf_t *cfg,
                             app_settings_store_t *store,
                             const app_settings_defaults_t *defaults,
                             const app_settings_t *initial,
                             app_cli_config_report_fn report,
                             void *report_ctx,
                             app_cli_config_apply_fn apply,
                             void *apply_ctx)
{
    if (!cfg) {
        return;
    }

    memset(cfg, 0, sizeof(*cfg));
    cfg->store = store;
    cfg->report = report;
    cfg->report_ctx = report_ctx;
    cfg->apply = apply;
    cfg->apply_ctx = apply_ctx;
    if (defaults) {
        cfg->defaults = *defaults;
    }
    if (initial) {
        cfg->current = *initial;
    } else {
        app_settings_defaults(&cfg->current, defaults);
    }
}

bool app_cli_config_idf_handle(app_cli_config_idf_t *cfg, const blinky_config_command_t *cmd)
{
    if (!cfg || !cmd) {
        return false;
    }

    switch (cmd->action) {
    case BLINKY_CONFIG_CMD_SHOW:
        switch (cmd->view) {
        case BLINKY_CONFIG_VIEW_STARTUP:
            report_show_startup(cfg);
            return true;
        case BLINKY_CONFIG_VIEW_LOGGING:
            report_show_logging(cfg);
            return true;
        case BLINKY_CONFIG_VIEW_ALL:
        case BLINKY_CONFIG_VIEW_NONE:
        default:
            report_show_all(cfg);
            return true;
        }

    case BLINKY_CONFIG_CMD_SET:
        switch (cmd->key) {
        case BLINKY_CONFIG_KEY_STARTUP_WAVE:
            if (cmd->value.kind != BLINKY_CONFIG_VALUE_STARTUP_WAVE) {
                report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config set failed: invalid startup wave value");
                return false;
            }
            cfg->current.startup_selector = startup_selector_from_command(cmd->value.as.startup_wave);
            report_set_result(cfg, "startup_wave", startup_wave_name(cfg->current.startup_selector));
            break;
        case BLINKY_CONFIG_KEY_BOOT_PATTERN:
            if (cmd->value.kind != BLINKY_CONFIG_VALUE_BOOL) {
                report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config set failed: invalid boot-pattern value");
                return false;
            }
            cfg->current.boot_pattern_enabled = cmd->value.as.bool_value;
            report_set_result(cfg, "boot_pattern", on_off_name(cfg->current.boot_pattern_enabled));
            break;
        case BLINKY_CONFIG_KEY_LOG_INTENSITY:
            if (cmd->value.kind != BLINKY_CONFIG_VALUE_BOOL) {
                report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config set failed: invalid log intensity value");
                return false;
            }
            cfg->current.log_intensity_enabled = cmd->value.as.bool_value;
            report_set_result(cfg, "log_intensity", on_off_name(cfg->current.log_intensity_enabled));
            break;
        case BLINKY_CONFIG_KEY_LOG_LEVEL:
            if (cmd->value.kind != BLINKY_CONFIG_VALUE_LOG_LEVEL) {
                report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config set failed: invalid log level value");
                return false;
            }
            cfg->current.log_min_level = cmd->value.as.log_level;
            report_set_result(cfg, "log_level", log_level_name(cfg->current.log_min_level));
            break;
        case BLINKY_CONFIG_KEY_NONE:
        default:
            report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config set failed: unsupported key");
            return false;
        }

        cfg->dirty = true;
        apply_current(cfg);
        return true;

    case BLINKY_CONFIG_CMD_SAVE:
        if (!cfg->store || !cfg->store->ops || !cfg->store->ops->save) {
            report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config save failed: store unavailable");
            return false;
        }
        if (cfg->store->ops->save(cfg->store->ctx, &cfg->current) != APP_SETTINGS_STORE_OK) {
            report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config save failed");
            return false;
        }
        cfg->dirty = false;
        report_line(cfg, BLINKY_LOG_LEVEL_INFO, "config saved");
        return true;

    case BLINKY_CONFIG_CMD_RESET:
        if (!cfg->store || !cfg->store->ops || !cfg->store->ops->reset) {
            report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config reset failed: store unavailable");
            return false;
        }
        if (cfg->store->ops->reset(cfg->store->ctx) != APP_SETTINGS_STORE_OK) {
            report_line(cfg, BLINKY_LOG_LEVEL_WARN, "config reset failed");
            return false;
        }
        app_settings_defaults(&cfg->current, &cfg->defaults);
        cfg->dirty = false;
        apply_current(cfg);
        report_line(cfg, BLINKY_LOG_LEVEL_INFO, "config reset to defaults");
        return true;

    case BLINKY_CONFIG_CMD_NONE:
    default:
        return false;
    }
}
