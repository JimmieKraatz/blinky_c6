#pragma once

#include "button_input_adapter_idf.h"
#include "led_core_config.h"
#include "led_output_adapter_idf.h"

typedef struct {
    led_output_adapter_idf_config_t led_output;
    gpio_num_t button_gpio;
    bool button_active_low;
    button_pull_t button_pull;
    blinky_time_ms_t producer_poll_ms;
    int boot_pattern_ms;
} led_platform_config_t;

void idf_build_platform_config(led_platform_config_t *cfg);
void idf_build_core_config(led_core_config_t *cfg);
