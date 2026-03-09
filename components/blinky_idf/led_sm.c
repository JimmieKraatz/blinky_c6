#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "menu_logic.h"
#include "led_sm.h"

#define LED_GPIO ((gpio_num_t)CONFIG_BLINKY_LED_GPIO)
#define BTN_GPIO ((gpio_num_t)CONFIG_BLINKY_BTN_GPIO)

#define POLL_MS CONFIG_BLINKY_POLL_MS
#define DEBOUNCE_COUNT CONFIG_BLINKY_DEBOUNCE_COUNT
#define LONG_PRESS_MS CONFIG_BLINKY_LONG_PRESS_MS

static inline blinky_time_ms_t now_ms(void)
{
    return (blinky_time_ms_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static blinky_event_t to_blinky_event(button_event_t ev)
{
    switch (ev) {
    case BUTTON_EVENT_SHORT_PRESS:
        return BLINKY_EVENT_SHORT_PRESS;
    case BUTTON_EVENT_LONG_PRESS:
        return BLINKY_EVENT_LONG_PRESS;
    case BUTTON_EVENT_NONE:
    default:
        return BLINKY_EVENT_NONE;
    }
}

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

#define LEDC_FREQ_HZ     CONFIG_BLINKY_PWM_FREQ_HZ

/* Local run/pause FSM for LED behavior. */
typedef enum {
    ST_LED_RUNNING = 0,
    ST_LED_PAUSED,
    ST_LED_MENU,
    ST_LED_COUNT
} sm_led_state_t;

static inline void led_write(sm_led_ctx_t *ctx, bool on)
{
    led_output_adapter_write(&ctx->led_output, on);
}

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

static const char *wave_name(led_wave_t wave)
{
    switch (wave) {
    case LED_WAVE_SQUARE:
        return "SQUARE";
    case LED_WAVE_SAW_UP:
        return "SAW_UP";
    case LED_WAVE_SAW_DOWN:
        return "SAW_DOWN";
    case LED_WAVE_TRIANGLE:
        return "TRIANGLE";
    case LED_WAVE_SINE:
        return "SINE";
    default:
        return "UNKNOWN";
    }
}

static void enter_running(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    printf("STATE: RUNNING\n");
    printf("SINE_STEPS_MAX=%d\n", CONFIG_BLINKY_SINE_STEPS_MAX);
    printf("SINE_STEPS_USED=%u\n", (unsigned)led_model_sine_steps(&led_ctx->model));
    led_model_set_wave(&led_ctx->model,
                       led_ctx->model.wave,
                       now_ms());
    led_write(led_ctx, false);
}

static fsm_state_t next_running(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    led_brightness_t brightness = 0;
    button_event_t ev = button_poll_event(&led_ctx->button, xTaskGetTickCount());
    blinky_event_t bev = to_blinky_event(ev);
    if (led_model_tick_raw(&led_ctx->model,
                           now_ms(),
                           &brightness)) {
        led_output_adapter_set_brightness(&led_ctx->led_output, brightness);
#if CONFIG_BLINKY_LOG_INTENSITY
        printf("LED %u%%\n", (unsigned)led_percent_from_brightness(brightness));
#endif
    }

    if (bev == BLINKY_EVENT_SHORT_PRESS) {
        return ST_LED_PAUSED;
    }
    if (bev == BLINKY_EVENT_LONG_PRESS) {
        led_ctx->menu_return_state = ST_LED_RUNNING;
        return ST_LED_MENU;
    }
    return ST_LED_RUNNING;
}

static void enter_paused(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    printf("STATE: PAUSED\n");
    printf("MODEL: %s\n", wave_name(led_ctx->model.wave));
    led_write(led_ctx, false);
}

static fsm_state_t next_paused(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    button_event_t ev = button_poll_event(&led_ctx->button, xTaskGetTickCount());
    blinky_event_t bev = to_blinky_event(ev);
    if (bev == BLINKY_EVENT_SHORT_PRESS) {
        return ST_LED_RUNNING;
    }
    if (bev == BLINKY_EVENT_LONG_PRESS) {
        led_ctx->menu_return_state = ST_LED_PAUSED;
        return ST_LED_MENU;
    }
    return ST_LED_PAUSED;
}

static void enter_menu(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    led_ctx->menu_wave = led_ctx->model.wave;
    printf("STATE: MENU\n");
    printf("MENU: WAVE %s\n", wave_name(led_ctx->menu_wave));
    led_model_set_wave(&led_ctx->model,
                       led_ctx->menu_wave,
                       now_ms());
}

static fsm_state_t next_menu(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    button_event_t ev = button_poll_event(&led_ctx->button, xTaskGetTickCount());
    blinky_event_t bev = to_blinky_event(ev);
    led_brightness_t brightness = 0;
    if (led_model_tick_raw(&led_ctx->model,
                           now_ms(),
                           &brightness)) {
        led_output_adapter_set_brightness(&led_ctx->led_output, brightness);
    }
    led_menu_action_t action = led_menu_handle_event(&led_ctx->menu_wave, bev);
    if (action == LED_MENU_ACTION_WAVE_CHANGED) {
        led_model_set_wave(&led_ctx->model,
                           led_ctx->menu_wave,
                           now_ms());
        printf("MENU: WAVE %s\n", wave_name(led_ctx->menu_wave));
    } else if (action == LED_MENU_ACTION_EXIT) {
        printf("MENU: EXIT\n");
        return (fsm_state_t)led_ctx->menu_return_state;
    }
    return ST_LED_MENU;
}

static const fsm_state_def_t STATE[ST_LED_COUNT] = {
    [ST_LED_RUNNING] = { .enter = enter_running, .next = next_running },
    [ST_LED_PAUSED] = { .enter = enter_paused, .next = next_paused },
    [ST_LED_MENU] = { .enter = enter_menu, .next = next_menu },
};

void led_sm_init(sm_led_ctx_t *ctx)
{
    /* Keep LED and button setup local to this module for now. */
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
    button_init(&ctx->button,
                BTN_GPIO,
                CONFIG_BLINKY_BTN_ACTIVE_LOW,
                pull,
                DEBOUNCE_COUNT,
                LONG_PRESS_MS);
    led_model_init(&ctx->model,
                   &(led_model_config_t){
                       .wave_period_ms = CONFIG_BLINKY_WAVE_PERIOD_MS,
                       .poll_ms = CONFIG_BLINKY_POLL_MS,
                       .sine_steps_max = CONFIG_BLINKY_SINE_STEPS_MAX,
                       .saw_step_pct = CONFIG_BLINKY_SAW_STEP_PCT,
                   });
    led_model_set_wave(&ctx->model, led_start_wave_from_config(), now_ms());
    led_show_startup_pattern(ctx, ctx->model.wave);
    fsm_enter(ctx, STATE, ST_LED_COUNT, ST_LED_RUNNING, true);
}

void led_sm_step(sm_led_ctx_t *ctx)
{
    vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    fsm_step(ctx, STATE, ST_LED_COUNT, false);
}
