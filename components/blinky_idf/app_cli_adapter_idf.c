#include "app_cli_adapter_idf.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "sdkconfig.h"

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"

#include "app_event_factory.h"

#define CLI_LINE_MAX_CHARS 96U
#define CLI_READ_BUDGET    64U

typedef struct {
    bool ready;
    char line[CLI_LINE_MAX_CHARS];
    size_t len;
} app_cli_adapter_state_t;

static app_cli_adapter_state_t s_cli = {0};
static const char *TAG = "blinky_cli";

static void normalize_line(const char *in, char *out, size_t out_sz)
{
    if (!in || !out || out_sz == 0U) {
        return;
    }

    size_t oi = 0U;
    bool have_content = false;
    bool space_pending = false;

    for (size_t i = 0U; in[i] != '\0' && oi + 1U < out_sz; ++i) {
        const unsigned char c = (unsigned char)in[i];
        if (isspace(c)) {
            if (have_content) {
                space_pending = true;
            }
            continue;
        }

        if (space_pending && oi + 1U < out_sz) {
            out[oi++] = ' ';
            space_pending = false;
        }
        out[oi++] = (char)tolower(c);
        have_content = true;
    }

    out[oi] = '\0';
}

static blinky_cli_command_t parse_command(const char *line)
{
    char normalized[CLI_LINE_MAX_CHARS] = {0};
    normalize_line(line, normalized, sizeof(normalized));

    if (normalized[0] == '\0') {
        return BLINKY_CLI_CMD_NONE;
    }
    if (strcmp(normalized, "help") == 0) {
        return BLINKY_CLI_CMD_HELP;
    }
    if (strcmp(normalized, "status") == 0) {
        return BLINKY_CLI_CMD_STATUS;
    }
    if (strcmp(normalized, "run") == 0 || strcmp(normalized, "pause") == 0
        || strcmp(normalized, "run pause toggle") == 0) {
        return BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE;
    }
    if (strcmp(normalized, "menu enter") == 0) {
        return BLINKY_CLI_CMD_MENU_ENTER;
    }
    if (strcmp(normalized, "menu next") == 0) {
        return BLINKY_CLI_CMD_MENU_NEXT;
    }
    if (strcmp(normalized, "menu exit") == 0) {
        return BLINKY_CLI_CMD_MENU_EXIT;
    }

    return BLINKY_CLI_CMD_NONE;
}

static void handle_line(sm_led_ctx_t *ctx, const char *line)
{
    if (!ctx || !line) {
        return;
    }
    bool has_non_space = false;
    for (size_t i = 0U; line[i] != '\0'; ++i) {
        if (!isspace((unsigned char)line[i])) {
            has_non_space = true;
            break;
        }
    }
    if (!has_non_space) {
        return;
    }

    const blinky_cli_command_t cmd = parse_command(line);
    switch (cmd) {
    case BLINKY_CLI_CMD_HELP:
        ESP_LOGI(TAG, "commands: help, status, run, pause, menu enter, menu next, menu exit");
        return;
    case BLINKY_CLI_CMD_STATUS:
        ESP_LOGI(TAG, "status: started=%s, dropped=%lu",
                 ctx->started ? "true" : "false",
                 (unsigned long)app_dispatcher_dropped(&ctx->dispatcher));
        return;
    case BLINKY_CLI_CMD_NONE:
        ESP_LOGW(TAG, "unknown command: '%s'", line);
        return;
    default:
        break;
    }

    const blinky_time_ms_t now = button_input_adapter_now_ms(&ctx->input);
    const app_event_t ev = app_event_factory_from_cli_command(cmd, now);
    if (ev.type == APP_EVENT_NONE) {
        return;
    }

    if (!led_sm_enqueue_event(ctx, &ev)) {
        ESP_LOGW(TAG, "failed to enqueue command '%s'", line);
    }
}

void app_cli_adapter_idf_init(sm_led_ctx_t *ctx)
{
    (void)ctx;
#if CONFIG_BLINKY_CLI_ENABLE
    const esp_err_t err = uart_driver_install(
        UART_NUM_0, CONFIG_BLINKY_CLI_UART_RX_BUF_SIZE, 0, 0, NULL, 0);
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        s_cli.ready = true;
        s_cli.len = 0U;
        ESP_LOGI(TAG, "cli adapter ready on UART0");
    } else {
        s_cli.ready = false;
        ESP_LOGW(TAG, "cli adapter disabled (uart_driver_install err=0x%x)", (unsigned)err);
    }
#endif
}

void app_cli_adapter_idf_step(sm_led_ctx_t *ctx)
{
    (void)ctx;
#if CONFIG_BLINKY_CLI_ENABLE
    if (!s_cli.ready || !ctx) {
        return;
    }

    for (size_t i = 0U; i < CLI_READ_BUDGET; ++i) {
        uint8_t ch = 0U;
        const int got = uart_read_bytes(UART_NUM_0, &ch, 1, 0);
        if (got <= 0) {
            break;
        }

        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            s_cli.line[s_cli.len] = '\0';
            handle_line(ctx, s_cli.line);
            s_cli.len = 0U;
            continue;
        }

        if (isprint((unsigned char)ch) && s_cli.len + 1U < sizeof(s_cli.line)) {
            s_cli.line[s_cli.len++] = (char)ch;
            continue;
        }

        if (s_cli.len + 1U >= sizeof(s_cli.line)) {
            s_cli.len = 0U;
            ESP_LOGW(TAG, "cli line too long; dropped");
        }
    }
#endif
}
