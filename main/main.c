#include "sdkconfig.h"
#include "led_sm_idf.h"

static sm_led_ctx_t s_led_ctx;

/* App entry point: delegate behavior to LED state machine module. */
void app_main(void)
{
    led_sm_init(&s_led_ctx);

    while (1) {
        led_sm_step(&s_led_ctx);
    }
}
