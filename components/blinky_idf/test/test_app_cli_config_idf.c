#include <string.h>

#include "unity.h"

#include "app_cli_config_idf.h"

typedef struct {
    app_settings_t saved;
    unsigned save_calls;
    unsigned reset_calls;
} fake_store_t;

typedef struct {
    char last_line[160];
    blinky_log_level_t last_level;
    unsigned count;
} fake_report_t;

typedef struct {
    app_settings_t last;
    unsigned count;
} fake_apply_t;

static app_settings_store_status_t fake_save(void *ctx, const app_settings_t *cfg)
{
    fake_store_t *store = (fake_store_t *)ctx;
    store->saved = *cfg;
    store->save_calls++;
    return APP_SETTINGS_STORE_OK;
}

static app_settings_store_status_t fake_reset(void *ctx)
{
    fake_store_t *store = (fake_store_t *)ctx;
    store->reset_calls++;
    return APP_SETTINGS_STORE_OK;
}

static void fake_report(void *ctx, blinky_log_level_t level, const char *line)
{
    fake_report_t *report = (fake_report_t *)ctx;
    report->last_level = level;
    report->count++;
    strncpy(report->last_line, line ? line : "", sizeof(report->last_line) - 1U);
    report->last_line[sizeof(report->last_line) - 1U] = '\0';
}

static void fake_apply(void *ctx, const app_settings_t *cfg)
{
    fake_apply_t *apply = (fake_apply_t *)ctx;
    apply->last = *cfg;
    apply->count++;
}

TEST_CASE("cli config setter updates in-memory settings and marks save pending", "[app_cli_config_idf]")
{
    fake_store_t store_ctx = {0};
    fake_report_t report = {0};
    fake_apply_t apply = {0};
    app_settings_store_t store = {
        .ops = &(app_settings_store_ops_t){
            .save = fake_save,
            .reset = fake_reset,
        },
        .ctx = &store_ctx,
    };
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = false,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};
    blinky_config_command_t cmd = {
        .action = BLINKY_CONFIG_CMD_SET,
        .key = BLINKY_CONFIG_KEY_BOOT_PATTERN,
        .value = {
            .kind = BLINKY_CONFIG_VALUE_BOOL,
            .as.bool_value = true,
        },
    };

    app_settings_defaults(&initial, &defaults);
    app_cli_config_idf_init(&cfg, &store, &defaults, &initial, fake_report, &report, fake_apply, &apply);

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(&cfg, &cmd));
    TEST_ASSERT_TRUE(cfg.current.boot_pattern_enabled);
    TEST_ASSERT_TRUE(cfg.dirty);
    TEST_ASSERT_EQUAL_UINT32(1U, apply.count);
    TEST_ASSERT_TRUE(apply.last.boot_pattern_enabled);
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "config updated: boot_pattern=on"));
}

TEST_CASE("cli config setters cover startup wave log intensity and log level", "[app_cli_config_idf]")
{
    fake_store_t store_ctx = {0};
    fake_report_t report = {0};
    fake_apply_t apply = {0};
    app_settings_store_t store = {
        .ops = &(app_settings_store_ops_t){
            .save = fake_save,
            .reset = fake_reset,
        },
        .ctx = &store_ctx,
    };
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = false,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};

    app_settings_defaults(&initial, &defaults);
    app_cli_config_idf_init(&cfg, &store, &defaults, &initial, fake_report, &report, fake_apply, &apply);

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(
        &cfg,
        &(blinky_config_command_t){
            .action = BLINKY_CONFIG_CMD_SET,
            .key = BLINKY_CONFIG_KEY_STARTUP_WAVE,
            .value = {
                .kind = BLINKY_CONFIG_VALUE_STARTUP_WAVE,
                .as.startup_wave = BLINKY_CONFIG_STARTUP_WAVE_TRIANGLE,
            },
        }));
    TEST_ASSERT_EQUAL_UINT32(LED_STARTUP_SELECT_TRIANGLE, (uint32_t)cfg.current.startup_selector);

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(
        &cfg,
        &(blinky_config_command_t){
            .action = BLINKY_CONFIG_CMD_SET,
            .key = BLINKY_CONFIG_KEY_LOG_INTENSITY,
            .value = {
                .kind = BLINKY_CONFIG_VALUE_BOOL,
                .as.bool_value = true,
            },
        }));
    TEST_ASSERT_TRUE(cfg.current.log_intensity_enabled);

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(
        &cfg,
        &(blinky_config_command_t){
            .action = BLINKY_CONFIG_CMD_SET,
            .key = BLINKY_CONFIG_KEY_LOG_LEVEL,
            .value = {
                .kind = BLINKY_CONFIG_VALUE_LOG_LEVEL,
                .as.log_level = BLINKY_LOG_LEVEL_ERROR,
            },
        }));
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_ERROR, (uint32_t)cfg.current.log_min_level);

    TEST_ASSERT_TRUE(cfg.dirty);
    TEST_ASSERT_EQUAL_UINT32(3U, apply.count);
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "config updated: log_level=error"));
}

TEST_CASE("cli config setter rejects mismatched value kind", "[app_cli_config_idf]")
{
    fake_report_t report = {0};
    fake_apply_t apply = {0};
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = false,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};

    app_settings_defaults(&initial, &defaults);
    app_cli_config_idf_init(&cfg, NULL, &defaults, &initial, fake_report, &report, fake_apply, &apply);

    TEST_ASSERT_FALSE(app_cli_config_idf_handle(
        &cfg,
        &(blinky_config_command_t){
            .action = BLINKY_CONFIG_CMD_SET,
            .key = BLINKY_CONFIG_KEY_LOG_LEVEL,
            .value = {
                .kind = BLINKY_CONFIG_VALUE_BOOL,
                .as.bool_value = true,
            },
        }));
    TEST_ASSERT_FALSE(cfg.dirty);
    TEST_ASSERT_EQUAL_UINT32(0U, apply.count);
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "invalid log level value"));
}

TEST_CASE("cli config save persists current settings and clears dirty flag", "[app_cli_config_idf]")
{
    fake_store_t store_ctx = {0};
    fake_report_t report = {0};
    app_settings_store_t store = {
        .ops = &(app_settings_store_ops_t){
            .save = fake_save,
            .reset = fake_reset,
        },
        .ctx = &store_ctx,
    };
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = false,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};
    blinky_config_command_t cmd = {
        .action = BLINKY_CONFIG_CMD_SAVE,
    };

    app_settings_defaults(&initial, &defaults);
    initial.log_min_level = BLINKY_LOG_LEVEL_DEBUG;
    app_cli_config_idf_init(&cfg, &store, &defaults, &initial, fake_report, &report, NULL, NULL);
    cfg.dirty = true;

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(&cfg, &cmd));
    TEST_ASSERT_EQUAL_UINT32(1U, store_ctx.save_calls);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_DEBUG, (uint32_t)store_ctx.saved.log_min_level);
    TEST_ASSERT_FALSE(cfg.dirty);
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "config saved"));
}

TEST_CASE("cli config reset clears store and restores default-backed settings", "[app_cli_config_idf]")
{
    fake_store_t store_ctx = {0};
    fake_report_t report = {0};
    fake_apply_t apply = {0};
    app_settings_store_t store = {
        .ops = &(app_settings_store_ops_t){
            .save = fake_save,
            .reset = fake_reset,
        },
        .ctx = &store_ctx,
    };
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = true,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_WARN,
        .startup_selector = LED_STARTUP_SELECT_TRIANGLE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};
    blinky_config_command_t cmd = {
        .action = BLINKY_CONFIG_CMD_RESET,
    };

    app_settings_defaults(&initial, &defaults);
    initial.boot_pattern_enabled = false;
    initial.log_min_level = BLINKY_LOG_LEVEL_DEBUG;
    app_cli_config_idf_init(&cfg, &store, &defaults, &initial, fake_report, &report, fake_apply, &apply);
    cfg.dirty = true;

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(&cfg, &cmd));
    TEST_ASSERT_EQUAL_UINT32(1U, store_ctx.reset_calls);
    TEST_ASSERT_FALSE(cfg.dirty);
    TEST_ASSERT_TRUE(cfg.current.boot_pattern_enabled);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_WARN, (uint32_t)cfg.current.log_min_level);
    TEST_ASSERT_EQUAL_UINT32(1U, apply.count);
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "config reset"));
}

TEST_CASE("cli config show reports current effective values", "[app_cli_config_idf]")
{
    fake_report_t report = {0};
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = false,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};
    blinky_config_command_t cmd = {
        .action = BLINKY_CONFIG_CMD_SHOW,
        .view = BLINKY_CONFIG_VIEW_LOGGING,
    };

    app_settings_defaults(&initial, &defaults);
    initial.log_intensity_enabled = true;
    initial.log_min_level = BLINKY_LOG_LEVEL_DEBUG;
    app_cli_config_idf_init(&cfg, NULL, &defaults, &initial, fake_report, &report, NULL, NULL);
    cfg.dirty = true;

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(&cfg, &cmd));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "config logging:"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "intensity=on"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "level=debug"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "pending_save=on"));
}

TEST_CASE("cli config show all reports startup and logging fields together", "[app_cli_config_idf]")
{
    fake_report_t report = {0};
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = false,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};
    blinky_config_command_t cmd = {
        .action = BLINKY_CONFIG_CMD_SHOW,
        .view = BLINKY_CONFIG_VIEW_ALL,
    };

    app_settings_defaults(&initial, &defaults);
    initial.boot_pattern_enabled = true;
    initial.log_intensity_enabled = true;
    initial.log_min_level = BLINKY_LOG_LEVEL_WARN;
    initial.startup_selector = LED_STARTUP_SELECT_TRIANGLE;
    app_cli_config_idf_init(&cfg, NULL, &defaults, &initial, fake_report, &report, NULL, NULL);

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(&cfg, &cmd));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "config:"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "startup_wave=triangle"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "boot_pattern=on"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "log_intensity=on"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "log_level=warn"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "pending_save=off"));
}

TEST_CASE("cli config show startup reports startup-specific fields", "[app_cli_config_idf]")
{
    fake_report_t report = {0};
    const app_settings_defaults_t defaults = {
        .boot_pattern_enabled = false,
        .log_intensity_enabled = false,
        .log_min_level = BLINKY_LOG_LEVEL_INFO,
        .startup_selector = LED_STARTUP_SELECT_SINE,
    };
    app_settings_t initial = {0};
    app_cli_config_idf_t cfg = {0};
    blinky_config_command_t cmd = {
        .action = BLINKY_CONFIG_CMD_SHOW,
        .view = BLINKY_CONFIG_VIEW_STARTUP,
    };

    app_settings_defaults(&initial, &defaults);
    initial.boot_pattern_enabled = true;
    initial.startup_selector = LED_STARTUP_SELECT_SAW_DOWN;
    app_cli_config_idf_init(&cfg, NULL, &defaults, &initial, fake_report, &report, NULL, NULL);
    cfg.dirty = true;

    TEST_ASSERT_TRUE(app_cli_config_idf_handle(&cfg, &cmd));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "config startup:"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "wave=saw_down"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "boot_pattern=on"));
    TEST_ASSERT_NOT_EQUAL(NULL, strstr(report.last_line, "pending_save=on"));
}
