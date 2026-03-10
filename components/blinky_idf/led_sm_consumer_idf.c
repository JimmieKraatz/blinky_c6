#include "led_sm_idf.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_SM_CONSUMER_TASK_STACK_WORDS 2048U
#define LED_SM_CONSUMER_TASK_PRIORITY    5U

static TaskHandle_t s_consumer_task = NULL;
static StaticTask_t s_consumer_task_tcb;
static StackType_t s_consumer_task_stack[LED_SM_CONSUMER_TASK_STACK_WORDS];

void led_sm_consumer_step(sm_led_ctx_t *ctx, size_t max_events)
{
    (void)app_dispatcher_drain(&ctx->dispatcher, max_events);
}

static void led_sm_consumer_task(void *arg)
{
    sm_led_ctx_t *ctx = (sm_led_ctx_t *)arg;

    for (;;) {
        (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        led_sm_consumer_step(ctx, 0);
    }
}

void led_sm_consumer_task_start(sm_led_ctx_t *ctx)
{
    if (!ctx || s_consumer_task) {
        return;
    }

    s_consumer_task = xTaskCreateStatic(
        led_sm_consumer_task,
        "led_consumer",
        LED_SM_CONSUMER_TASK_STACK_WORDS,
        ctx,
        LED_SM_CONSUMER_TASK_PRIORITY,
        s_consumer_task_stack,
        &s_consumer_task_tcb);
}

void led_sm_consumer_task_stop(sm_led_ctx_t *ctx)
{
    (void)ctx;
    if (!s_consumer_task) {
        return;
    }
    vTaskDelete(s_consumer_task);
    s_consumer_task = NULL;
}

void led_sm_consumer_task_notify(sm_led_ctx_t *ctx)
{
    (void)ctx;
    if (!s_consumer_task) {
        return;
    }
    xTaskNotifyGive(s_consumer_task);
}
