#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include "led_output_adapter.h"

typedef struct {
    gpio_num_t gpio;
    uint32_t pwm_freq_hz;
    bool active_low;
} led_output_adapter_idf_config_t;

typedef struct {
    ledc_mode_t speed_mode;
    ledc_channel_t channel;
    ledc_timer_t timer;
    ledc_timer_bit_t duty_resolution;
} led_output_adapter_idf_t;

esp_err_t led_output_adapter_idf_init(led_output_adapter_t *adapter,
                                      led_output_adapter_idf_t *impl,
                                      const led_output_adapter_idf_config_t *cfg);
