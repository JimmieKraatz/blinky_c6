#include "button_logic.h"

#include <assert.h>

void button_logic_init(button_logic_t *logic,
                       button_logic_debounce_t debounce_count,
                       blinky_time_ms_t long_press_ms)
{
    assert(logic);

    logic->stable = false;
    logic->debounce = 0;
    logic->debounce_count = debounce_count;
    logic->pressed_at_ms = 0;
    logic->long_press_ms = long_press_ms;
    logic->long_reported = false;
}

button_logic_event_t button_logic_poll_raw(button_logic_t *logic,
                                           bool raw_pressed,
                                           blinky_time_ms_t now_ms)
{
    assert(logic);

    if (raw_pressed != logic->stable) {
        if (++logic->debounce >= logic->debounce_count) {
            bool prev = logic->stable;
            logic->stable = raw_pressed;
            logic->debounce = 0;
            if (!prev && logic->stable) {
                logic->pressed_at_ms = now_ms;
                logic->long_reported = false;
            } else if (prev && !logic->stable) {
                if (!logic->long_reported && (now_ms - logic->pressed_at_ms) < logic->long_press_ms) {
                    return BUTTON_LOGIC_EVENT_SHORT_PRESS;
                }
            }
        }
    } else {
        logic->debounce = 0;
    }

    if (logic->stable && !logic->long_reported &&
        (now_ms - logic->pressed_at_ms) >= logic->long_press_ms) {
        logic->long_reported = true;
        return BUTTON_LOGIC_EVENT_LONG_PRESS;
    }

    return BUTTON_LOGIC_EVENT_NONE;
}
