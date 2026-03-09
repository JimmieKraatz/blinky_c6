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

static void enter_running(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    printf("STATE: RUNNING\n");
    printf("SINE_STEPS_MAX=%d\n", CONFIG_BLINKY_SINE_STEPS_MAX);
    printf("SINE_STEPS_USED=%u\n", (unsigned)led_model_sine_steps(&led_ctx->model));
    led_model_set_wave(&led_ctx->model,
                       led_ctx->model.wave,
                       button_input_adapter_now_ms(&led_ctx->input));
    led_write(led_ctx, false);
}

static fsm_state_t next_running(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    led_brightness_t brightness = 0;
    blinky_event_t bev = button_input_adapter_poll_event(&led_ctx->input);
    led_policy_step_result_t policy = {0};

    if (led_model_tick_raw(&led_ctx->model,
                           button_input_adapter_now_ms(&led_ctx->input),
                           &brightness)) {
        led_output_adapter_set_brightness(&led_ctx->led_output, brightness);
#if CONFIG_BLINKY_LOG_INTENSITY
        printf("LED %u%%\n", (unsigned)led_percent_from_brightness(brightness));
#endif
    }

    policy = led_policy_step(&led_ctx->policy, LED_POLICY_RUNNING, led_ctx->model.wave, bev);
    return (fsm_state_t)policy.next_state;
}

static void enter_paused(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    printf("STATE: PAUSED\n");
    printf("MODEL: %s\n", led_policy_wave_name(led_ctx->model.wave));
    led_write(led_ctx, false);
}

static fsm_state_t next_paused(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    blinky_event_t bev = button_input_adapter_poll_event(&led_ctx->input);
    led_policy_step_result_t policy = led_policy_step(&led_ctx->policy, LED_POLICY_PAUSED, led_ctx->model.wave, bev);
    return (fsm_state_t)policy.next_state;
}

static void enter_menu(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    printf("STATE: MENU\n");
    printf("MENU: WAVE %s\n", led_policy_wave_name(led_ctx->policy.menu_wave));
    led_model_set_wave(&led_ctx->model,
                       led_ctx->policy.menu_wave,
                       button_input_adapter_now_ms(&led_ctx->input));
}

static fsm_state_t next_menu(void *ctx)
{
    sm_led_ctx_t *led_ctx = (sm_led_ctx_t *)ctx;
    blinky_event_t bev = button_input_adapter_poll_event(&led_ctx->input);
    led_brightness_t brightness = 0;
    led_policy_step_result_t policy = {0};

    if (led_model_tick_raw(&led_ctx->model,
                           button_input_adapter_now_ms(&led_ctx->input),
                           &brightness)) {
        led_output_adapter_set_brightness(&led_ctx->led_output, brightness);
    }

    policy = led_policy_step(&led_ctx->policy, LED_POLICY_MENU, led_ctx->model.wave, bev);
    if (policy.menu_wave_changed) {
        led_model_set_wave(&led_ctx->model,
                           led_ctx->policy.menu_wave,
                           button_input_adapter_now_ms(&led_ctx->input));
        printf("MENU: WAVE %s\n", led_policy_wave_name(led_ctx->policy.menu_wave));
    }
    if (policy.next_state != LED_POLICY_MENU) {
        printf("MENU: EXIT\n");
    }
    return (fsm_state_t)policy.next_state;
}

static const fsm_state_def_t STATE[LED_POLICY_COUNT] = {
    [LED_POLICY_RUNNING] = { .enter = enter_running, .next = next_running },
    [LED_POLICY_PAUSED] = { .enter = enter_paused, .next = next_paused },
    [LED_POLICY_MENU] = { .enter = enter_menu, .next = next_menu },
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
    led_model_init(&ctx->model,
                   &(led_model_config_t){
                       .wave_period_ms = CONFIG_BLINKY_WAVE_PERIOD_MS,
                       .poll_ms = CONFIG_BLINKY_POLL_MS,
                       .sine_steps_max = CONFIG_BLINKY_SINE_STEPS_MAX,
                       .saw_step_pct = CONFIG_BLINKY_SAW_STEP_PCT,
                   });
    led_model_set_wave(&ctx->model, led_start_wave_from_config(), button_input_adapter_now_ms(&ctx->input));
    ctx->policy.menu_return_state = LED_POLICY_RUNNING;
    ctx->policy.menu_wave = ctx->model.wave;
    led_show_startup_pattern(ctx, ctx->model.wave);
    fsm_enter(ctx, STATE, LED_POLICY_COUNT, LED_POLICY_RUNNING, true);
}

void led_sm_step(sm_led_ctx_t *ctx)
{
    vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    fsm_step(ctx, STATE, LED_POLICY_COUNT, false);
}
