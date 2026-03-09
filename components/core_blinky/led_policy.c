#include "led_policy.h"

#include "menu_logic.h"

led_policy_step_result_t led_policy_step(led_policy_ctx_t *ctx,
                                         led_policy_state_t current_state,
                                         led_wave_t current_wave,
                                         blinky_event_t event)
{
    led_policy_step_result_t out = {
        .next_state = current_state,
        .menu_wave_changed = false,
    };

    switch (current_state) {
    case LED_POLICY_RUNNING:
        if (event == BLINKY_EVENT_SHORT_PRESS) {
            out.next_state = LED_POLICY_PAUSED;
        } else if (event == BLINKY_EVENT_LONG_PRESS) {
            ctx->menu_return_state = LED_POLICY_RUNNING;
            ctx->menu_wave = current_wave;
            out.next_state = LED_POLICY_MENU;
        }
        break;

    case LED_POLICY_PAUSED:
        if (event == BLINKY_EVENT_SHORT_PRESS) {
            out.next_state = LED_POLICY_RUNNING;
        } else if (event == BLINKY_EVENT_LONG_PRESS) {
            ctx->menu_return_state = LED_POLICY_PAUSED;
            ctx->menu_wave = current_wave;
            out.next_state = LED_POLICY_MENU;
        }
        break;

    case LED_POLICY_MENU: {
        led_menu_action_t action = led_menu_handle_event(&ctx->menu_wave, event);
        if (action == LED_MENU_ACTION_WAVE_CHANGED) {
            out.menu_wave_changed = true;
        } else if (action == LED_MENU_ACTION_EXIT) {
            out.next_state = ctx->menu_return_state;
        }
        break;
    }

    default:
        break;
    }

    return out;
}

const char *led_policy_wave_name(led_wave_t wave)
{
    switch (wave) {
    case LED_WAVE_SQUARE:
        return "SQUARE";
    case LED_WAVE_SAW_UP:
        return "SAW_UP";
    case LED_WAVE_SAW_DOWN:
        return "SAW_DOWN";
    case LED_WAVE_TRIANGLE:
        return "TRIANGLE";
    case LED_WAVE_SINE:
        return "SINE";
    default:
        return "UNKNOWN";
    }
}
