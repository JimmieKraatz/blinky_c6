#pragma once

#include <stdbool.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#include "button_idf.h"
#include "button_input_adapter.h"

typedef struct {
    gpio_num_t gpio;
    bool active_low;
    button_pull_t pull;
    button_logic_debounce_t debounce_count;
    blinky_time_ms_t long_press_ms;
} button_input_adapter_idf_config_t;

typedef struct {
    button_t button;
} button_input_adapter_idf_t;

void button_input_adapter_idf_init(button_input_adapter_t *adapter,
                                   button_input_adapter_idf_t *impl,
                                   const button_input_adapter_idf_config_t *cfg);
