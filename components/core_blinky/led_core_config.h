#pragma once

#include "button_policy.h"
#include "led_model.h"
#include "led_startup_policy.h"

typedef struct {
    led_model_config_t model;
    led_startup_config_t startup;
    button_policy_timing_t button_timing;
} led_core_config_t;
