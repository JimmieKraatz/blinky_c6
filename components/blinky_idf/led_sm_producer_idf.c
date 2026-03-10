#include "led_sm_idf.h"

static app_event_type_t app_event_from_blinky_event(blinky_event_t ev)
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

void led_sm_producer_step(sm_led_ctx_t *ctx)
{
    blinky_time_ms_t now = button_input_adapter_now_ms(&ctx->input);
    blinky_event_t bev = button_input_adapter_poll_event(&ctx->input);
    app_event_type_t type = app_event_from_blinky_event(bev);

    (void)app_event_queue_push(&ctx->queue, &(app_event_t){
        .type = type,
        .timestamp_ms = now,
        .payload = {.u32 = 0},
    });
}
