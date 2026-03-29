#include "led_command_dispatch.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *runtime_state_name(led_policy_state_t state)
{
    switch (state) {
    case LED_POLICY_RUNNING:
        return "running";
    case LED_POLICY_PAUSED:
        return "paused";
    case LED_POLICY_MENU:
        return "menu";
    default:
        return "unknown";
    }
}

static const char *command_name(blinky_control_command_t cmd)
{
    switch (cmd) {
    case BLINKY_CONTROL_CMD_RUN:
        return "run";
    case BLINKY_CONTROL_CMD_PAUSE:
        return "pause";
    case BLINKY_CONTROL_CMD_RUN_PAUSE_TOGGLE:
        return "run_pause_toggle";
    case BLINKY_CONTROL_CMD_MENU_ENTER:
        return "menu_enter";
    case BLINKY_CONTROL_CMD_MENU_NEXT:
        return "menu_next";
    case BLINKY_CONTROL_CMD_MENU_EXIT:
        return "menu_exit";
    case BLINKY_CONTROL_CMD_NONE:
    default:
        return "none";
    }
}

static void log_command_ignored(led_runtime_t *rt, blinky_control_command_t cmd)
{
    if (!rt || !rt->log_sink) {
        return;
    }

    char message[64] = {0};
    (void)snprintf(message, sizeof(message), "ignored in state=%s", runtime_state_name(rt->state));
    const blinky_log_kv_t kv = blinky_log_kv_str("command", command_name(cmd));
    const blinky_log_record_t record = {
        .level = BLINKY_LOG_LEVEL_INFO,
        .domain = "runtime",
        .event = "command",
        .message = message,
        .kvs = &kv,
        .kv_count = 1,
    };
    blinky_log_emit(rt->log_sink, &record);
}

static bool command_to_input_event(led_policy_state_t state,
                                   blinky_control_command_t cmd,
                                   blinky_event_t *out)
{
    if (!out) {
        return false;
    }

    switch (cmd) {
    case BLINKY_CONTROL_CMD_RUN:
        if (state == LED_POLICY_PAUSED) {
            *out = BLINKY_EVENT_SHORT_PRESS;
            return true;
        }
        return false;
    case BLINKY_CONTROL_CMD_PAUSE:
        if (state == LED_POLICY_RUNNING) {
            *out = BLINKY_EVENT_SHORT_PRESS;
            return true;
        }
        return false;
    case BLINKY_CONTROL_CMD_RUN_PAUSE_TOGGLE:
        if (state == LED_POLICY_RUNNING || state == LED_POLICY_PAUSED) {
            *out = BLINKY_EVENT_SHORT_PRESS;
            return true;
        }
        return false;
    case BLINKY_CONTROL_CMD_MENU_ENTER:
        if (state == LED_POLICY_RUNNING || state == LED_POLICY_PAUSED) {
            *out = BLINKY_EVENT_LONG_PRESS;
            return true;
        }
        return false;
    case BLINKY_CONTROL_CMD_MENU_NEXT:
        if (state == LED_POLICY_MENU) {
            *out = BLINKY_EVENT_SHORT_PRESS;
            return true;
        }
        return false;
    case BLINKY_CONTROL_CMD_MENU_EXIT:
        if (state == LED_POLICY_MENU) {
            *out = BLINKY_EVENT_LONG_PRESS;
            return true;
        }
        return false;
    case BLINKY_CONTROL_CMD_NONE:
    default:
        return false;
    }
}

led_command_result_t led_command_dispatch(led_runtime_t *rt,
                                          blinky_control_command_t cmd,
                                          blinky_time_ms_t now,
                                          led_runtime_output_t *out)
{
    assert(rt);
    assert(out);

    memset(out, 0, sizeof(*out));

    blinky_event_t input = BLINKY_EVENT_NONE;
    if (!command_to_input_event(rt->state, cmd, &input)) {
        log_command_ignored(rt, cmd);
        return cmd == BLINKY_CONTROL_CMD_NONE ? LED_COMMAND_RESULT_INVALID : LED_COMMAND_RESULT_IGNORED;
    }

    led_runtime_step(rt, now, input, out);
    return LED_COMMAND_RESULT_APPLIED;
}
