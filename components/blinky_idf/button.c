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
                 button_pull_t pull,
                 button_logic_debounce_t debounce_count,
                 blinky_time_ms_t long_press_ms)
{
    gpio_pullup_t pull_up = GPIO_PULLUP_DISABLE;
    gpio_pulldown_t pull_down = GPIO_PULLDOWN_DISABLE;
    if (pull == BUTTON_PULL_UP) {
        pull_up = GPIO_PULLUP_ENABLE;
    } else if (pull == BUTTON_PULL_DOWN) {
        pull_down = GPIO_PULLDOWN_ENABLE;
    }

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = pull_up,
        .pull_down_en = pull_down,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);

    btn->gpio = gpio;
    btn->active_low = active_low;
    button_logic_init(&btn->logic, debounce_count, long_press_ms);
}

static button_event_t to_button_event(button_logic_event_t ev)
{
    switch (ev) {
    case BUTTON_LOGIC_EVENT_SHORT_PRESS:
        return BUTTON_EVENT_SHORT_PRESS;
    case BUTTON_LOGIC_EVENT_LONG_PRESS:
        return BUTTON_EVENT_LONG_PRESS;
    case BUTTON_LOGIC_EVENT_NONE:
    default:
        return BUTTON_EVENT_NONE;
    }
}

button_event_t button_poll_event(button_t *btn, TickType_t now)
{
    blinky_time_ms_t now_ms = (blinky_time_ms_t)(now * portTICK_PERIOD_MS);
    return to_button_event(button_logic_poll_raw(&btn->logic, button_raw_pressed(btn), now_ms));
}

button_event_t button_poll_event_raw(button_t *btn, bool raw_pressed, TickType_t now)
{
    blinky_time_ms_t now_ms = (blinky_time_ms_t)(now * portTICK_PERIOD_MS);
    return to_button_event(button_logic_poll_raw(&btn->logic, raw_pressed, now_ms));
}
