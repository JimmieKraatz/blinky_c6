#pragma once

#include "blinky_log.h"

typedef struct {
    const char *tag;
    blinky_log_level_t min_level;
} blinky_log_adapter_idf_config_t;

typedef struct {
    const char *tag;
    blinky_log_level_t min_level;
} blinky_log_adapter_idf_t;

void blinky_log_adapter_idf_init(blinky_log_sink_t *sink,
                                 blinky_log_adapter_idf_t *adapter,
                                 const blinky_log_adapter_idf_config_t *cfg);
