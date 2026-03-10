#include "led_sm_idf.h"

void led_sm_consumer_step(sm_led_ctx_t *ctx, size_t max_events)
{
    (void)app_dispatcher_drain(&ctx->dispatcher, max_events);
}
