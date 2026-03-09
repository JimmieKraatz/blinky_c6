#include "led_output_adapter_idf.h"

#include <stddef.h>

#include "driver/ledc.h"
#include "esp_check.h"

#define LEDC_TIMER_NUM   LEDC_TIMER_0
#define LEDC_SPEED_MODE  LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_NUM LEDC_CHANNEL_0
#define LEDC_DUTY_RES    LEDC_TIMER_13_BIT

static inline uint32_t ledc_max_duty(ledc_timer_bit_t resolution)
{
    return (1U << resolution) - 1U;
}

static inline uint32_t duty_from_brightness(ledc_timer_bit_t resolution, led_brightness_t brightness)
{
    return ((((uint32_t)brightness) * ledc_max_duty(resolution)) + (LED_BRIGHTNESS_MAX / 2U)) / LED_BRIGHTNESS_MAX;
}

static void idf_set_brightness(void *ctx, led_brightness_t brightness)
{
    led_output_adapter_idf_t *impl = (led_output_adapter_idf_t *)ctx;
    uint32_t duty = duty_from_brightness(impl->duty_resolution, brightness);
    ledc_set_duty(impl->speed_mode, impl->channel, duty);
    ledc_update_duty(impl->speed_mode, impl->channel);
}

static void idf_set_percent(void *ctx, led_percent_t percent)
{
    idf_set_brightness(ctx, led_brightness_from_percent(percent));
}

static const led_output_adapter_ops_t IDF_OPS = {
    .set_brightness = idf_set_brightness,
    .set_percent = idf_set_percent,
};

esp_err_t led_output_adapter_idf_init(led_output_adapter_t *adapter,
                                      led_output_adapter_idf_t *impl,
                                      const led_output_adapter_idf_config_t *cfg)
{
    if (!adapter || !impl || !cfg) {
        return ESP_ERR_INVALID_ARG;
    }

    impl->speed_mode = LEDC_SPEED_MODE;
    impl->channel = LEDC_CHANNEL_NUM;
    impl->timer = LEDC_TIMER_NUM;
    impl->duty_resolution = LEDC_DUTY_RES;

    ledc_timer_config_t tcfg = {
        .speed_mode = impl->speed_mode,
        .timer_num = impl->timer,
        .duty_resolution = impl->duty_resolution,
        .freq_hz = cfg->pwm_freq_hz,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&tcfg));

    ledc_channel_config_t ccfg = {
        .gpio_num = cfg->gpio,
        .speed_mode = impl->speed_mode,
        .channel = impl->channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = impl->timer,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = cfg->active_low ? 1 : 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ccfg));
    ESP_ERROR_CHECK(ledc_set_duty(impl->speed_mode, impl->channel, 0));
    ESP_ERROR_CHECK(ledc_update_duty(impl->speed_mode, impl->channel));

    adapter->ops = &IDF_OPS;
    adapter->ctx = impl;
    return ESP_OK;
}
