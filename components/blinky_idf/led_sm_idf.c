#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_config_idf.h"
#include "led_sm_idf.h"
#include "led_event_consumer.h"
#include "led_event_factory.h"
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
            printf("LED %u%%\n", (unsigned)led_percent_from_brightness(out->brightness));
        }
    }
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

void led_sm_init(sm_led_ctx_t *ctx)
{
    idf_build_platform_config(&ctx->platform_cfg);
    idf_build_core_config(&ctx->core_cfg);

    ESP_ERROR_CHECK(led_output_adapter_idf_init(
        &ctx->led_output,
        &ctx->led_output_idf,
        &ctx->platform_cfg.led_output));

    button_input_adapter_idf_init(
        &ctx->input,
        &ctx->input_idf,
        &(button_input_adapter_idf_config_t){
            .gpio = ctx->platform_cfg.button_gpio,
            .active_low = ctx->platform_cfg.button_active_low,
            .pull = ctx->platform_cfg.button_pull,
            .timing = ctx->core_cfg.button_timing,
        });

    led_runtime_output_t out = {0};
    led_runtime_init(
        &ctx->runtime,
        &ctx->core_cfg.model,
        led_startup_policy_select_wave(&ctx->core_cfg.startup),
        button_input_adapter_now_ms(&ctx->input),
        &out);

    app_event_queue_init(&ctx->queue);
    ctx->sink_ops = &QUEUE_SINK_OPS;
    ctx->sink_ctx = ctx;
    app_dispatcher_init(
        &ctx->dispatcher,
        &QUEUE_SOURCE_OPS,
        &ctx->queue,
        dispatch_event,
        ctx);
    ctx->started = false;
    led_sm_start(ctx);

    apply_runtime_output(ctx, &out);
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
}

void led_sm_start(sm_led_ctx_t *ctx)
{
    if (!ctx || ctx->started) {
        return;
    }

    led_sm_consumer_task_start(ctx);

    /* Seed boot into the same producer/consumer pipeline used at runtime. */
    app_event_t boot = led_event_factory_boot(button_input_adapter_now_ms(&ctx->input));
    (void)led_sm_enqueue_event(ctx, &boot);

    led_show_startup_pattern(ctx, ctx->runtime.model.wave);
    ctx->started = true;
}

void led_sm_stop(sm_led_ctx_t *ctx)
{
    if (!ctx || !ctx->started) {
        return;
    }

    led_sm_consumer_task_stop(ctx);
    ctx->started = false;
}
