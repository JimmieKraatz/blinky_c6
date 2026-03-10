#pragma once

#include "button_input_adapter_idf.h"
#include "led_output_adapter_idf.h"
#include "led_runtime.h"
#include "led_startup_policy.h"

typedef struct {
    led_output_adapter_idf_config_t led_output;
    button_input_adapter_idf_config_t button_input;
    blinky_time_ms_t producer_poll_ms;
    int boot_pattern_ms;
} led_platform_config_t;

typedef struct {
    led_model_config_t model;
    led_startup_config_t startup;
} led_core_config_t;

void idf_build_platform_config(led_platform_config_t *cfg);
void idf_build_core_config(led_core_config_t *cfg);
