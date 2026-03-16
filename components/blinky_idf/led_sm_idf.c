#include <stdbool.h>
#include <stdint.h>

#include "esp_app_desc.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_config_idf.h"
#include "led_sm_idf.h"
#include "led_event_consumer.h"
#include "app_event_factory.h"
#include "app_cli_adapter_idf.h"
#include "app_settings_store_idf.h"
#include "led_startup_policy.h"

static inline void led_write(sm_led_ctx_t *ctx, bool on)
{
    led_output_adapter_write(&ctx->led_output, on);
}

static void apply_runtime_output(sm_led_ctx_t *ctx, const led_runtime_output_t *out)
{
    if (out->write_level) {
        led_write(ctx, out->level_on);
    }
    if (out->write_brightness) {
        led_output_adapter_set_brightness(&ctx->led_output, out->brightness);
        if (ctx->platform_cfg.log_intensity_enabled) {
            const blinky_log_kv_t kv = blinky_log_kv_uint(
                "percent", (uint32_t)led_percent_from_brightness(out->brightness));
            const blinky_log_record_t record = {
                .level = BLINKY_LOG_LEVEL_INFO,
                .domain = "led",
                .event = "intensity",
                .message = "brightness update",
                .kvs = &kv,
                .kv_count = 1,
            };
            blinky_log_emit(&ctx->log_sink, &record);
        }
    }
}

static const char *log_level_name(blinky_log_level_t level)
{
    switch (level) {
    case BLINKY_LOG_LEVEL_ERROR:
        return "ERROR";
    case BLINKY_LOG_LEVEL_WARN:
        return "WARN";
    case BLINKY_LOG_LEVEL_DEBUG:
        return "DEBUG";
    case BLINKY_LOG_LEVEL_INFO:
    default:
        return "INFO";
    }
}

static void log_boot_identity(sm_led_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    const esp_app_desc_t *app = esp_app_get_description();
    const blinky_log_kv_t kvs[] = {
        blinky_log_kv_str("project", app ? app->project_name : ""),
        blinky_log_kv_str("version", app ? app->version : ""),
        blinky_log_kv_str("idf", app ? app->idf_ver : ""),
        blinky_log_kv_str("min_level", log_level_name(ctx->platform_cfg.log_min_level)),
    };
    const blinky_log_record_t record = {
        .level = BLINKY_LOG_LEVEL_INFO,
        .domain = "app",
        .event = "boot",
        .message = "startup",
        .kvs = kvs,
        .kv_count = 4,
    };
    blinky_log_emit(&ctx->log_sink, &record);
}

static void log_settings_probe(sm_led_ctx_t *ctx,
                               blinky_log_level_t level,
                               const char *message,
                               const char *source,
                               const app_settings_t *cfg)
{
    if (!ctx) {
        return;
    }

    blinky_log_kv_t kvs[6] = {
        blinky_log_kv_str("source", source ? source : ""),
        blinky_log_kv_uint("schema_ver", cfg ? cfg->schema_version : 0U),
        blinky_log_kv_uint("boot_pattern", (cfg && cfg->boot_pattern_enabled) ? 1U : 0U),
        blinky_log_kv_uint("log_intensity", (cfg && cfg->log_intensity_enabled) ? 1U : 0U),
        blinky_log_kv_str("log_level", cfg ? log_level_name(cfg->log_min_level) : ""),
        blinky_log_kv_uint("test_count", cfg ? cfg->test_counter : 0U),
    };
    const blinky_log_record_t record = {
        .level = level,
        .domain = "settings",
        .event = "probe",
        .message = message,
        .kvs = kvs,
        .kv_count = 6,
    };
    blinky_log_emit(&ctx->log_sink, &record);
}

static app_settings_defaults_t app_settings_platform_defaults(const sm_led_ctx_t *ctx)
{
    return (app_settings_defaults_t){
        .boot_pattern_enabled = ctx ? ctx->platform_cfg.boot_pattern_enabled : false,
        .log_intensity_enabled = ctx ? ctx->platform_cfg.log_intensity_enabled : false,
        .log_min_level = ctx ? ctx->platform_cfg.log_min_level : BLINKY_LOG_LEVEL_INFO,
    };
}

static void app_settings_apply_platform_overrides(sm_led_ctx_t *ctx, const app_settings_t *cfg)
{
    if (!ctx || !cfg) {
        return;
    }

    ctx->platform_cfg.boot_pattern_enabled = cfg->boot_pattern_enabled;
    ctx->platform_cfg.log_intensity_enabled = cfg->log_intensity_enabled;
    ctx->platform_cfg.log_min_level = cfg->log_min_level;
    ctx->log_idf.min_level = cfg->log_min_level;
}

static void app_settings_probe(sm_led_ctx_t *ctx)
{
    if (!ctx || !ctx->settings_store.ops || !ctx->settings_store.ops->load ||
        !ctx->settings_store.ops->save) {
        return;
    }

    app_settings_t cfg = {0};
    const app_settings_defaults_t defaults = app_settings_platform_defaults(ctx);
    app_settings_store_status_t status = ctx->settings_store.ops->load(ctx->settings_store.ctx, &cfg);
    if (status == APP_SETTINGS_STORE_OK) {
        app_settings_apply_platform_overrides(ctx, &cfg);
        log_settings_probe(ctx, BLINKY_LOG_LEVEL_INFO, "loaded placeholder settings", "persisted", &cfg);
        return;
    }

    if (status != APP_SETTINGS_STORE_ERR_NOT_FOUND) {
        log_settings_probe(ctx, BLINKY_LOG_LEVEL_WARN, "placeholder settings load failed", "error", NULL);
        return;
    }

    app_settings_defaults(&cfg, &defaults);
    status = ctx->settings_store.ops->save(ctx->settings_store.ctx, &cfg);
    if (status != APP_SETTINGS_STORE_OK) {
        log_settings_probe(ctx, BLINKY_LOG_LEVEL_WARN, "placeholder settings seed failed", "seed", &cfg);
        return;
    }

    status = ctx->settings_store.ops->load(ctx->settings_store.ctx, &cfg);
    if (status != APP_SETTINGS_STORE_OK) {
        log_settings_probe(ctx, BLINKY_LOG_LEVEL_WARN, "placeholder settings verify failed", "seed", NULL);
        return;
    }

    app_settings_apply_platform_overrides(ctx, &cfg);
    log_settings_probe(ctx, BLINKY_LOG_LEVEL_INFO, "seeded placeholder settings", "seed", &cfg);
}

static void dispatch_event(void *ctx, const app_event_t *ev)
{
    sm_led_ctx_t *sm = (sm_led_ctx_t *)ctx;
    led_runtime_output_t out = {0};
    led_event_consumer_dispatch(&sm->runtime, ev, &out);
    apply_runtime_output(sm, &out);
}

static bool queue_pop(void *ctx, app_event_t *out)
{
    app_event_queue_t *q = (app_event_queue_t *)ctx;
    return app_event_queue_pop(q, out);
}

static bool queue_push(void *ctx, const app_event_t *ev)
{
    sm_led_ctx_t *sm = (sm_led_ctx_t *)ctx;
    if (!app_event_queue_push(&sm->queue, ev)) {
        return false;
    }
    led_sm_consumer_task_notify(sm);
    return true;
}

static uint32_t queue_dropped(void *ctx)
{
    app_event_queue_t *q = (app_event_queue_t *)ctx;
    return app_event_queue_dropped(q);
}

static const app_event_sink_ops_t QUEUE_SINK_OPS = {
    .push = queue_push,
};

static const app_event_source_ops_t QUEUE_SOURCE_OPS = {
    .pop = queue_pop,
    .dropped = queue_dropped,
};

static void led_show_startup_pattern(sm_led_ctx_t *ctx, led_wave_t wave)
{
    if (ctx->platform_cfg.boot_pattern_enabled) {
        int count = (int)wave + 1;
        for (int i = 0; i < count; ++i) {
            led_write(ctx, true);
            vTaskDelay(pdMS_TO_TICKS(ctx->platform_cfg.boot_pattern_ms));
            led_write(ctx, false);
            vTaskDelay(pdMS_TO_TICKS(ctx->platform_cfg.boot_pattern_ms));
        }
    }
}

static void led_runtime_reinitialize(sm_led_ctx_t *ctx)
{
    led_runtime_output_t out = {0};
    blinky_time_ms_t now = button_input_adapter_now_ms(&ctx->input);
    led_runtime_init(
        &ctx->runtime,
        &ctx->core_cfg.model,
        led_startup_policy_select_wave(&ctx->core_cfg.startup),
        now,
        &out);
    apply_runtime_output(ctx, &out);
}

void led_sm_init(sm_led_ctx_t *ctx)
{
    idf_build_platform_config(&ctx->platform_cfg);
    idf_build_core_config(&ctx->core_cfg);

    ESP_ERROR_CHECK(led_output_adapter_idf_init(
        &ctx->led_output,
        &ctx->led_output_idf,
        &ctx->platform_cfg.led_output));
    blinky_log_adapter_idf_init(
        &ctx->log_sink,
        &ctx->log_idf,
        &(blinky_log_adapter_idf_config_t){
            .tag = "blinky",
            .min_level = ctx->platform_cfg.log_min_level,
        });
    log_boot_identity(ctx);

    button_input_adapter_idf_init(
        &ctx->input,
        &ctx->input_idf,
        &(button_input_adapter_idf_config_t){
            .gpio = ctx->platform_cfg.button_gpio,
            .active_low = ctx->platform_cfg.button_active_low,
            .pull = ctx->platform_cfg.button_pull,
            .timing = ctx->core_cfg.button_timing,
        });

    (void)app_settings_store_idf_init(
        &ctx->settings_store,
        &ctx->settings_store_idf,
        &(app_settings_store_idf_config_t){
            .partition_label = ctx->platform_cfg.settings_partition_label,
            .namespace_name = ctx->platform_cfg.settings_namespace,
        });
    app_settings_probe(ctx);

    led_runtime_set_log_sink(&ctx->runtime, &ctx->log_sink);

    app_event_queue_init(&ctx->queue);
    ctx->sink_ops = &QUEUE_SINK_OPS;
    ctx->sink_ctx = ctx;
    app_dispatcher_init(
        &ctx->dispatcher,
        &QUEUE_SOURCE_OPS,
        &ctx->queue,
        dispatch_event,
        ctx);
    app_cli_adapter_idf_init(ctx);
    ctx->started = false;
    (void)led_sm_start(ctx, LED_SM_START_FRESH);
}

bool led_sm_enqueue_event(sm_led_ctx_t *ctx, const app_event_t *ev)
{
    if (!ctx || !ctx->sink_ops || !ctx->sink_ops->push) {
        return false;
    }
    return ctx->sink_ops->push(ctx->sink_ctx, ev);
}

void led_sm_step(sm_led_ctx_t *ctx)
{
    vTaskDelay(pdMS_TO_TICKS(ctx->platform_cfg.producer_poll_ms));
    led_sm_producer_step(ctx);
    app_cli_adapter_idf_step(ctx);
}

bool led_sm_start(sm_led_ctx_t *ctx, led_sm_start_mode_t mode)
{
    if (!ctx) {
        return false;
    }
    if (ctx->started) {
        return true;
    }

    if (mode == LED_SM_START_FRESH) {
        app_event_queue_init(&ctx->queue);
        app_dispatcher_init(
            &ctx->dispatcher,
            &QUEUE_SOURCE_OPS,
            &ctx->queue,
            dispatch_event,
            ctx);
        led_runtime_reinitialize(ctx);
        /* Ensure startup LED pattern is serialized before async consumer begins output writes. */
        led_show_startup_pattern(ctx, ctx->runtime.model.wave);
    }

    led_sm_consumer_task_start(ctx);
    if (!ctx->consumer_task) {
        return false;
    }

    if (mode == LED_SM_START_FRESH) {
        /* Seed boot into the same producer/consumer pipeline used at runtime. */
        app_event_t boot = app_event_factory_boot(button_input_adapter_now_ms(&ctx->input));
        (void)led_sm_enqueue_event(ctx, &boot);
    }
    ctx->started = true;
    return true;
}

void led_sm_stop(sm_led_ctx_t *ctx)
{
    if (!ctx || !ctx->started) {
        return;
    }

    led_sm_consumer_task_stop(ctx);
    ctx->started = false;
}
