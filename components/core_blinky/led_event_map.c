#include "led_event_map.h"

app_event_type_t led_event_map_from_blinky(blinky_event_t ev)
{
    switch (ev) {
    case BLINKY_EVENT_SHORT_PRESS:
        return APP_EVENT_BUTTON_SHORT;
    case BLINKY_EVENT_LONG_PRESS:
        return APP_EVENT_BUTTON_LONG;
    case BLINKY_EVENT_NONE:
    default:
        return APP_EVENT_TICK;
    }
}

blinky_event_t led_event_map_to_blinky(app_event_type_t type)
{
    switch (type) {
    case APP_EVENT_BUTTON_SHORT:
        return BLINKY_EVENT_SHORT_PRESS;
    case APP_EVENT_BUTTON_LONG:
        return BLINKY_EVENT_LONG_PRESS;
    case APP_EVENT_TICK:
    case APP_EVENT_BOOT:
    default:
        return BLINKY_EVENT_NONE;
    }
}

bool led_event_map_is_dispatchable(app_event_type_t type)
{
    switch (type) {
    case APP_EVENT_BOOT:
    case APP_EVENT_TICK:
    case APP_EVENT_BUTTON_SHORT:
    case APP_EVENT_BUTTON_LONG:
        return true;
    default:
        return false;
    }
}
