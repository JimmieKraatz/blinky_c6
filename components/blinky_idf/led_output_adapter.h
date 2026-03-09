#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "led_model.h"

/* Generic LED output adapter interface.
 * Hardware/framework-specific backends bind these operations at init time.
 */
typedef struct {
    void (*set_brightness)(void *ctx, led_brightness_t brightness);
    void (*set_percent)(void *ctx, led_percent_t percent);
} led_output_adapter_ops_t;

typedef struct {
    const led_output_adapter_ops_t *ops;
    void *ctx;
} led_output_adapter_t;

static inline void led_output_adapter_set_brightness(led_output_adapter_t *adapter,
                                                     led_brightness_t brightness)
{
    if (adapter && adapter->ops && adapter->ops->set_brightness) {
        adapter->ops->set_brightness(adapter->ctx, brightness);
    }
}

static inline void led_output_adapter_set_percent(led_output_adapter_t *adapter,
                                                  led_percent_t percent)
{
    if (adapter && adapter->ops && adapter->ops->set_percent) {
        adapter->ops->set_percent(adapter->ctx, percent);
    }
}

static inline void led_output_adapter_write(led_output_adapter_t *adapter, bool on)
{
    led_output_adapter_set_percent(adapter, on ? 100 : 0);
}
