#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "led_sm_idf.h"
#include "led_event_consumer.h"

#define LED_GPIO ((gpio_num_t)CONFIG_BLINKY_LED_GPIO)
#define BTN_GPIO ((gpio_num_t)CONFIG_BLINKY_BTN_GPIO)

#define POLL_MS CONFIG_BLINKY_POLL_MS
#define DEBOUNCE_COUNT CONFIG_BLINKY_DEBOUNCE_COUNT
#define LONG_PRESS_MS CONFIG_BLINKY_LONG_PRESS_MS

#define LEDC_FREQ_HZ CONFIG_BLINKY_PWM_FREQ_HZ

static led_wave_t led_start_wave_from_config(void)
{
#if CONFIG_BLINKY_START_WAVE_SQUARE
    return LED_WAVE_SQUARE;
#elif CONFIG_BLINKY_START_WAVE_SAW_UP
    return LED_WAVE_SAW_UP;
#elif CONFIG_BLINKY_START_WAVE_SAW_DOWN
    return LED_WAVE_SAW_DOWN;
#elif CONFIG_BLINKY_START_WAVE_TRIANGLE
    return LED_WAVE_TRIANGLE;
#else
    return LED_WAVE_SINE;
#endif
}

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
#if CONFIG_BLINKY_LOG_INTENSITY
        printf("LED %u%%\n", (unsigned)led_percent_from_brightness(out->brightness));
#endif
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

static uint32_t queue_dropped(void *ctx)
{
    app_event_queue_t *q = (app_event_queue_t *)ctx;
    return app_event_queue_dropped(q);
}

static const app_event_source_ops_t QUEUE_SOURCE_OPS = {
    .pop = queue_pop,
    .dropped = queue_dropped,
};

static void led_show_startup_pattern(sm_led_ctx_t *ctx, led_wave_t wave)
{
#if CONFIG_BLINKY_BOOT_PATTERN
    int count = (int)wave + 1;
    for (int i = 0; i < count; ++i) {
        led_write(ctx, true);
        vTaskDelay(pdMS_TO_TICKS(CONFIG_BLINKY_BOOT_PATTERN_MS));
        led_write(ctx, false);
        vTaskDelay(pdMS_TO_TICKS(CONFIG_BLINKY_BOOT_PATTERN_MS));
    }
#else
    (void)ctx;
    (void)wave;
#endif
}

void led_sm_init(sm_led_ctx_t *ctx)
{
    ESP_ERROR_CHECK(led_output_adapter_idf_init(
        &ctx->led_output,
        &ctx->led_output_idf,
        &(led_output_adapter_idf_config_t){
            .gpio = LED_GPIO,
            .pwm_freq_hz = LEDC_FREQ_HZ,
            .active_low = true,
        }));

    button_pull_t pull = BUTTON_PULL_NONE;
#if CONFIG_BLINKY_BTN_PULL_UP
    pull = BUTTON_PULL_UP;
#elif CONFIG_BLINKY_BTN_PULL_DOWN
    pull = BUTTON_PULL_DOWN;
#endif

    button_input_adapter_idf_init(
        &ctx->input,
        &ctx->input_idf,
        &(button_input_adapter_idf_config_t){
            .gpio = BTN_GPIO,
            .active_low = CONFIG_BLINKY_BTN_ACTIVE_LOW,
            .pull = pull,
            .debounce_count = DEBOUNCE_COUNT,
            .long_press_ms = LONG_PRESS_MS,
        });

    led_runtime_output_t out = {0};
    led_runtime_init(
        &ctx->runtime,
        &(led_model_config_t){
            .wave_period_ms = CONFIG_BLINKY_WAVE_PERIOD_MS,
            .poll_ms = CONFIG_BLINKY_POLL_MS,
            .sine_steps_max = CONFIG_BLINKY_SINE_STEPS_MAX,
            .saw_step_pct = CONFIG_BLINKY_SAW_STEP_PCT,
        },
        led_start_wave_from_config(),
        button_input_adapter_now_ms(&ctx->input),
        &out);

    app_event_queue_init(&ctx->queue);
    app_dispatcher_init(
        &ctx->dispatcher,
        &QUEUE_SOURCE_OPS,
        &ctx->queue,
        dispatch_event,
        ctx);
    led_sm_consumer_task_start(ctx);

    /* Seed boot into the same producer/consumer pipeline used at runtime. */
    if (app_event_queue_push(&ctx->queue, &(app_event_t){
        .type = APP_EVENT_BOOT,
        .timestamp_ms = button_input_adapter_now_ms(&ctx->input),
        .payload = {.u32 = 0},
    })) {
        led_sm_consumer_task_notify(ctx);
    }

    led_show_startup_pattern(ctx, ctx->runtime.model.wave);
    apply_runtime_output(ctx, &out);
}

void led_sm_step(sm_led_ctx_t *ctx)
{
    vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    led_sm_producer_step(ctx);
}
