#include "app_cli_adapter_idf.h"

#include <ctype.h>
#include <stdbool.h>

#include "sdkconfig.h"

#include "driver/uart.h"
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
#include "driver/usb_serial_jtag.h"
#endif
#include "esp_err.h"
#include "esp_log.h"

#include "app_event_factory.h"
#include "app_cli_parse.h"

#define CLI_LINE_MAX_CHARS 96U
#define CLI_READ_BUDGET    64U

typedef struct {
    bool ready;
    char line[CLI_LINE_MAX_CHARS];
    size_t len;
} app_cli_adapter_state_t;

static app_cli_adapter_state_t s_cli = {0};
static const char *TAG = "blinky_cli";

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

static const char *command_name(blinky_cli_command_t cmd)
{
    switch (cmd) {
    case BLINKY_CLI_CMD_HELP:
        return "help";
    case BLINKY_CLI_CMD_STATUS:
        return "status";
    case BLINKY_CLI_CMD_RUN:
        return "run";
    case BLINKY_CLI_CMD_PAUSE:
        return "pause";
    case BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE:
        return "run_pause_toggle";
    case BLINKY_CLI_CMD_MENU_ENTER:
        return "menu_enter";
    case BLINKY_CLI_CMD_MENU_NEXT:
        return "menu_next";
    case BLINKY_CLI_CMD_MENU_EXIT:
        return "menu_exit";
    case BLINKY_CLI_CMD_NONE:
    default:
        return "none";
    }
}

static bool should_dispatch(sm_led_ctx_t *ctx, blinky_cli_command_t cmd)
{
    switch (cmd) {
    case BLINKY_CLI_CMD_RUN:
        return ctx->runtime.state == LED_POLICY_PAUSED;
    case BLINKY_CLI_CMD_PAUSE:
        return ctx->runtime.state == LED_POLICY_RUNNING;
    case BLINKY_CLI_CMD_RUN_PAUSE_TOGGLE:
        return ctx->runtime.state == LED_POLICY_RUNNING || ctx->runtime.state == LED_POLICY_PAUSED;
    default:
        return true;
    }
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

    const blinky_cli_command_t cmd = app_cli_parse_line(line);
    switch (cmd) {
    case BLINKY_CLI_CMD_HELP:
        ESP_LOGI(TAG, "commands: help, status, run, pause, menu enter, menu next, menu exit");
        return;
    case BLINKY_CLI_CMD_STATUS:
        ESP_LOGI(TAG, "status: started=%s, state=%s, dropped=%lu",
                 ctx->started ? "true" : "false",
                 runtime_state_name(ctx->runtime.state),
                 (unsigned long)app_dispatcher_dropped(&ctx->dispatcher));
        return;
    case BLINKY_CLI_CMD_NONE:
        ESP_LOGW(TAG, "unknown command: '%s'", line);
        return;
    default:
        break;
    }

    if (!should_dispatch(ctx, cmd)) {
        ESP_LOGI(TAG, "command ignored in state=%s: '%s'", runtime_state_name(ctx->runtime.state), line);
        return;
    }

    ESP_LOGI(TAG, "cmd dispatch: %s", command_name(cmd));

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
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
    usb_serial_jtag_driver_config_t cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    cfg.rx_buffer_size = CONFIG_BLINKY_CLI_UART_RX_BUF_SIZE;
    const esp_err_t err = usb_serial_jtag_driver_install(&cfg);
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        s_cli.ready = true;
        s_cli.len = 0U;
        ESP_LOGI(TAG, "cli adapter ready on USB Serial/JTAG");
    } else {
        s_cli.ready = false;
        ESP_LOGW(TAG, "cli adapter disabled (usb_serial_jtag_driver_install err=0x%x)",
                 (unsigned)err);
    }
#else
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
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
        const int got = usb_serial_jtag_read_bytes(&ch, 1, 0);
#else
        const int got = uart_read_bytes(UART_NUM_0, &ch, 1, 0);
#endif
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
