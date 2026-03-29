#pragma once

#include <stdbool.h>

#include "blinky_log.h"

typedef enum {
    BLINKY_CONFIG_CMD_NONE = 0,
    BLINKY_CONFIG_CMD_SHOW,
    BLINKY_CONFIG_CMD_SET,
    BLINKY_CONFIG_CMD_SAVE,
    BLINKY_CONFIG_CMD_RESET,
} blinky_config_command_action_t;

typedef enum {
    BLINKY_CONFIG_VIEW_NONE = 0,
    BLINKY_CONFIG_VIEW_ALL,
    BLINKY_CONFIG_VIEW_STARTUP,
    BLINKY_CONFIG_VIEW_LOGGING,
} blinky_config_view_t;

typedef enum {
    BLINKY_CONFIG_KEY_NONE = 0,
    BLINKY_CONFIG_KEY_STARTUP_WAVE,
    BLINKY_CONFIG_KEY_BOOT_PATTERN,
    BLINKY_CONFIG_KEY_LOG_INTENSITY,
    BLINKY_CONFIG_KEY_LOG_LEVEL,
} blinky_config_key_t;

typedef enum {
    BLINKY_CONFIG_VALUE_NONE = 0,
    BLINKY_CONFIG_VALUE_BOOL,
    BLINKY_CONFIG_VALUE_LOG_LEVEL,
    BLINKY_CONFIG_VALUE_STARTUP_WAVE,
} blinky_config_value_kind_t;

typedef enum {
    BLINKY_CONFIG_STARTUP_WAVE_NONE = 0,
    BLINKY_CONFIG_STARTUP_WAVE_SQUARE,
    BLINKY_CONFIG_STARTUP_WAVE_SAW_UP,
    BLINKY_CONFIG_STARTUP_WAVE_SAW_DOWN,
    BLINKY_CONFIG_STARTUP_WAVE_TRIANGLE,
    BLINKY_CONFIG_STARTUP_WAVE_SINE,
} blinky_config_startup_wave_t;

typedef struct {
    blinky_config_value_kind_t kind;
    union {
        bool bool_value;
        blinky_log_level_t log_level;
        blinky_config_startup_wave_t startup_wave;
    } as;
} blinky_config_value_t;

typedef struct {
    blinky_config_command_action_t action;
    blinky_config_view_t view;
    blinky_config_key_t key;
    blinky_config_value_t value;
} blinky_config_command_t;
