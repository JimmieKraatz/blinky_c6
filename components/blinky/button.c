#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#include "button.h"

static inline bool button_raw_pressed(const button_t *btn)
{
    bool level = (gpio_get_level(btn->gpio) != 0);
    return btn->active_low ? !level : level;
}

void button_init(button_t *btn,
                 gpio_num_t gpio,
                 bool active_low,
                 uint8_t debounce_count,
                 uint32_t long_press_ms)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = active_low ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = active_low ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);

    btn->gpio = gpio;
    btn->active_low = active_low;
    btn->stable = false;
    btn->debounce = 0;
    btn->debounce_count = debounce_count;
    btn->pressed_at = 0;
    btn->long_press_ticks = pdMS_TO_TICKS(long_press_ms);
    btn->long_reported = false;
}

static button_event_t button_poll_event_impl(button_t *btn, bool raw, TickType_t now)
{
    if (raw != btn->stable) {
        if (++btn->debounce >= btn->debounce_count) {
            bool prev = btn->stable;
            btn->stable = raw;
            btn->debounce = 0;
            if (!prev && btn->stable) {
                btn->pressed_at = now;
                btn->long_reported = false;
            } else if (prev && !btn->stable) {
                if (!btn->long_reported && (now - btn->pressed_at) < btn->long_press_ticks) {
                    return BUTTON_EVENT_SHORT_PRESS;
                }
            }
        }
    } else {
        btn->debounce = 0;
    }

    if (btn->stable && !btn->long_reported &&
        (now - btn->pressed_at) >= btn->long_press_ticks) {
        btn->long_reported = true;
        return BUTTON_EVENT_LONG_PRESS;
    }

    return BUTTON_EVENT_NONE;
}

button_event_t button_poll_event(button_t *btn, TickType_t now)
{
    return button_poll_event_impl(btn, button_raw_pressed(btn), now);
}

button_event_t button_poll_event_raw(button_t *btn, bool raw_pressed, TickType_t now)
{
    return button_poll_event_impl(btn, raw_pressed, now);
}
