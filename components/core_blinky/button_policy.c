#include "button_policy.h"

#define BUTTON_POLICY_DEFAULT_DEBOUNCE ((button_logic_debounce_t)1U)
#define BUTTON_POLICY_DEFAULT_LONG_MS  ((blinky_time_ms_t)3000U)

button_policy_timing_t button_policy_timing_normalize(button_policy_timing_t timing)
{
    if (timing.debounce_count == 0U) {
        timing.debounce_count = BUTTON_POLICY_DEFAULT_DEBOUNCE;
    }
    if (timing.long_press_ms == 0U) {
        timing.long_press_ms = BUTTON_POLICY_DEFAULT_LONG_MS;
    }
    return timing;
}
