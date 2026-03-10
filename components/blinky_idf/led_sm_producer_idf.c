#include "led_sm_idf.h"
#include "led_event_map.h"

void led_sm_producer_step(sm_led_ctx_t *ctx)
{
    blinky_time_ms_t now = button_input_adapter_now_ms(&ctx->input);
    blinky_event_t bev = button_input_adapter_poll_event(&ctx->input);
    app_event_type_t type = led_event_map_from_blinky(bev);

    if (app_event_queue_push(&ctx->queue, &(app_event_t){
        .type = type,
        .timestamp_ms = now,
        .payload = {.u32 = 0},
    })) {
        led_sm_consumer_task_notify(ctx);
    }
}
