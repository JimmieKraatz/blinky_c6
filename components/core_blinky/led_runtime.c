#include "led_runtime.h"

#include <string.h>

static void clear_output(led_runtime_output_t *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

static void runtime_log(led_runtime_t *rt,
                        blinky_log_level_t level,
                        const char *event,
                        const char *message,
                        const blinky_log_kv_t *kvs,
                        uint8_t kv_count)
{
    if (!rt || !rt->log_sink) {
        return;
    }
    const blinky_log_record_t record = {
        .level = level,
        .domain = "runtime",
        .event = event,
        .message = message,
        .kvs = kvs,
        .kv_count = kv_count,
    };
    blinky_log_emit(rt->log_sink, &record);
}

static void enter_state(led_runtime_t *rt,
                        led_policy_state_t next,
                        blinky_time_ms_t now,
                        led_runtime_output_t *out)
{
    rt->state = next;
    switch (next) {
    case LED_POLICY_RUNNING: {
        const blinky_log_kv_t kv =
            blinky_log_kv_uint("sine_steps_used", led_model_sine_steps(&rt->model));
        runtime_log(rt, BLINKY_LOG_LEVEL_INFO, "state_change", "running", &kv, 1);
        led_model_set_wave(&rt->model, rt->model.wave, now);
        out->write_level = true;
        out->level_on = false;
        break;
    }

    case LED_POLICY_PAUSED: {
        const blinky_log_kv_t kv = blinky_log_kv_str("wave", led_policy_wave_name(rt->model.wave));
        runtime_log(rt, BLINKY_LOG_LEVEL_INFO, "state_change", "paused", &kv, 1);
        out->write_level = true;
        out->level_on = false;
        break;
    }

    case LED_POLICY_MENU: {
        const blinky_log_kv_t kv =
            blinky_log_kv_str("menu_wave", led_policy_wave_name(rt->policy.menu_wave));
        runtime_log(rt, BLINKY_LOG_LEVEL_INFO, "state_change", "menu", &kv, 1);
        led_model_set_wave(&rt->model, rt->policy.menu_wave, now);
        break;
    }

    default:
        break;
    }
}

void led_runtime_init(led_runtime_t *rt,
                      const led_model_config_t *cfg,
                      led_wave_t start_wave,
                      blinky_time_ms_t now,
                      led_runtime_output_t *out)
{
    blinky_log_sink_t *existing_sink = rt ? rt->log_sink : NULL;
    clear_output(out);
    led_model_init(&rt->model, cfg);
    led_model_set_wave(&rt->model, start_wave, now);
    rt->policy.menu_return_state = LED_POLICY_RUNNING;
    rt->policy.menu_wave = start_wave;
    rt->log_sink = existing_sink;
    enter_state(rt, LED_POLICY_RUNNING, now, out);
}

void led_runtime_step(led_runtime_t *rt,
                      blinky_time_ms_t now,
                      blinky_event_t event,
                      led_runtime_output_t *out)
{
    clear_output(out);

    if (rt->state == LED_POLICY_RUNNING || rt->state == LED_POLICY_MENU) {
        led_brightness_t brightness = 0;
        if (led_model_tick_raw(&rt->model, now, &brightness)) {
            out->write_brightness = true;
            out->brightness = brightness;
        }
    }

    led_policy_step_result_t step = led_policy_step(&rt->policy, rt->state, rt->model.wave, event);
    if (rt->state == LED_POLICY_MENU && step.menu_wave_changed) {
        led_model_set_wave(&rt->model, rt->policy.menu_wave, now);
        const blinky_log_kv_t kv =
            blinky_log_kv_str("menu_wave", led_policy_wave_name(rt->policy.menu_wave));
        runtime_log(rt, BLINKY_LOG_LEVEL_INFO, "menu_wave", "changed", &kv, 1);
    }

    if (step.next_state != rt->state) {
        if (rt->state == LED_POLICY_MENU) {
            runtime_log(rt, BLINKY_LOG_LEVEL_INFO, "menu", "exit", NULL, 0);
        }
        enter_state(rt, step.next_state, now, out);
    }
}

void led_runtime_set_log_sink(led_runtime_t *rt, blinky_log_sink_t *sink)
{
    if (!rt) {
        return;
    }
    rt->log_sink = sink;
}
