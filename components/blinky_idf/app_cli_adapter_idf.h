#pragma once

#include "led_sm_idf.h"

/* Initialize IDF CLI adapter resources. Safe to call once during startup. */
void app_cli_adapter_idf_init(sm_led_ctx_t *ctx);

/* Poll UART/console input, parse complete lines, and publish app events. */
void app_cli_adapter_idf_step(sm_led_ctx_t *ctx);
