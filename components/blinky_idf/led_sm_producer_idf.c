#include "led_sm_idf.h"
#include "led_event_factory.h"

void led_sm_producer_step(sm_led_ctx_t *ctx)
{
    blinky_time_ms_t now = button_input_adapter_now_ms(&ctx->input);
    blinky_event_t bev = button_input_adapter_poll_event(&ctx->input);
    app_event_t ev = led_event_factory_from_input(bev, now);

    if (led_sm_enqueue_event(ctx, &ev)) {
        led_sm_consumer_task_notify(ctx);
    }
}
