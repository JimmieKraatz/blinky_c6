#include "sdkconfig.h"
#include "led_sm_idf.h"

/* App entry point: delegate behavior to LED state machine module. */
#if !CONFIG_UNITY_ENABLE_IDF_TEST_RUNNER
void app_main(void)
{
    sm_led_ctx_t ctx = {0};

    led_sm_init(&ctx);

    while (1) {
        led_sm_step(&ctx);
    }
}
#endif
