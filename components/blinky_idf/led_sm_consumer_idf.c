#include "led_sm_idf.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

__attribute__((weak)) TaskHandle_t led_sm_consumer_task_create_static(TaskFunction_t task_code,
                                                                       const char *const name,
                                                                       const uint32_t stack_depth,
                                                                       void *const parameters,
                                                                       UBaseType_t priority,
                                                                       StackType_t *const stack_buffer,
                                                                       StaticTask_t *const task_buffer)
{
    return xTaskCreateStatic(task_code,
                             name,
                             stack_depth,
                             parameters,
                             priority,
                             stack_buffer,
                             task_buffer);
}

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
    if (!ctx || ctx->consumer_task) {
        return;
    }

    ctx->consumer_task = led_sm_consumer_task_create_static(
        led_sm_consumer_task,
        "led_consumer",
        LED_SM_CONSUMER_TASK_STACK_WORDS,
        ctx,
        LED_SM_CONSUMER_TASK_PRIORITY,
        ctx->consumer_task_stack,
        &ctx->consumer_task_tcb);
}

void led_sm_consumer_task_stop(sm_led_ctx_t *ctx)
{
    if (!ctx || !ctx->consumer_task) {
        return;
    }
    vTaskDelete(ctx->consumer_task);
    ctx->consumer_task = NULL;
}

void led_sm_consumer_task_notify(sm_led_ctx_t *ctx)
{
    if (!ctx || !ctx->consumer_task) {
        return;
    }
    xTaskNotifyGive(ctx->consumer_task);
}
