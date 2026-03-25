#include "app_cli_parse.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define CLI_LINE_MAX_CHARS 96U

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

static bool parse_bool_token(const char *token, bool *out)
{
    if (!token || !out) {
        return false;
    }
    if (strcmp(token, "on") == 0) {
        *out = true;
        return true;
    }
    if (strcmp(token, "off") == 0) {
        *out = false;
        return true;
    }
    return false;
}

static bool parse_log_level_token(const char *token, blinky_log_level_t *out)
{
    if (!token || !out) {
        return false;
    }
    if (strcmp(token, "error") == 0) {
        *out = BLINKY_LOG_LEVEL_ERROR;
        return true;
    }
    if (strcmp(token, "warn") == 0) {
        *out = BLINKY_LOG_LEVEL_WARN;
        return true;
    }
    if (strcmp(token, "info") == 0) {
        *out = BLINKY_LOG_LEVEL_INFO;
        return true;
    }
    if (strcmp(token, "debug") == 0) {
        *out = BLINKY_LOG_LEVEL_DEBUG;
        return true;
    }
    return false;
}

static bool parse_startup_wave_token(const char *token, blinky_config_startup_wave_t *out)
{
    if (!token || !out) {
        return false;
    }
    if (strcmp(token, "square") == 0) {
        *out = BLINKY_CONFIG_STARTUP_WAVE_SQUARE;
        return true;
    }
    if (strcmp(token, "saw_up") == 0) {
        *out = BLINKY_CONFIG_STARTUP_WAVE_SAW_UP;
        return true;
    }
    if (strcmp(token, "saw_down") == 0) {
        *out = BLINKY_CONFIG_STARTUP_WAVE_SAW_DOWN;
        return true;
    }
    if (strcmp(token, "triangle") == 0) {
        *out = BLINKY_CONFIG_STARTUP_WAVE_TRIANGLE;
        return true;
    }
    if (strcmp(token, "sine") == 0) {
        *out = BLINKY_CONFIG_STARTUP_WAVE_SINE;
        return true;
    }
    return false;
}

blinky_cli_command_t app_cli_parse_line(const char *line)
{
    char normalized[CLI_LINE_MAX_CHARS] = {0};
    normalize_line(line, normalized, sizeof(normalized));

    if (normalized[0] == '\0') {
        return BLINKY_CLI_CMD_NONE;
    }
    if (strcmp(normalized, "help") == 0) {
        return BLINKY_CLI_CMD_HELP;
    }
    if (strcmp(normalized, "help config") == 0) {
        return BLINKY_CLI_CMD_HELP_CONFIG;
    }
    if (strcmp(normalized, "status") == 0) {
        return BLINKY_CLI_CMD_STATUS;
    }
    if (strcmp(normalized, "run") == 0) {
        return BLINKY_CLI_CMD_RUN;
    }
    if (strcmp(normalized, "pause") == 0) {
        return BLINKY_CLI_CMD_PAUSE;
    }
    if (strcmp(normalized, "run pause toggle") == 0) {
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

bool app_cli_parse_config_command(const char *line, blinky_config_command_t *out)
{
    char normalized[CLI_LINE_MAX_CHARS] = {0};
    blinky_config_command_t parsed = {0};

    if (!out) {
        return false;
    }

    *out = parsed;
    normalize_line(line, normalized, sizeof(normalized));
    if (normalized[0] == '\0') {
        return false;
    }

    if (strcmp(normalized, "config show") == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SHOW;
        parsed.view = BLINKY_CONFIG_VIEW_ALL;
        *out = parsed;
        return true;
    }
    if (strcmp(normalized, "config show startup") == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SHOW;
        parsed.view = BLINKY_CONFIG_VIEW_STARTUP;
        *out = parsed;
        return true;
    }
    if (strcmp(normalized, "config show logging") == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SHOW;
        parsed.view = BLINKY_CONFIG_VIEW_LOGGING;
        *out = parsed;
        return true;
    }
    if (strcmp(normalized, "config save") == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SAVE;
        *out = parsed;
        return true;
    }
    if (strcmp(normalized, "config reset") == 0) {
        parsed.action = BLINKY_CONFIG_CMD_RESET;
        *out = parsed;
        return true;
    }

    const char *wave_prefix = "config startup wave ";
    const size_t wave_prefix_len = strlen(wave_prefix);
    if (strncmp(normalized, wave_prefix, wave_prefix_len) == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SET;
        parsed.key = BLINKY_CONFIG_KEY_STARTUP_WAVE;
        parsed.value.kind = BLINKY_CONFIG_VALUE_STARTUP_WAVE;
        if (parse_startup_wave_token(normalized + wave_prefix_len, &parsed.value.as.startup_wave)) {
            *out = parsed;
            return true;
        }
        return false;
    }

    const char *boot_prefix = "config boot-pattern ";
    const size_t boot_prefix_len = strlen(boot_prefix);
    if (strncmp(normalized, boot_prefix, boot_prefix_len) == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SET;
        parsed.key = BLINKY_CONFIG_KEY_BOOT_PATTERN;
        parsed.value.kind = BLINKY_CONFIG_VALUE_BOOL;
        if (parse_bool_token(normalized + boot_prefix_len, &parsed.value.as.bool_value)) {
            *out = parsed;
            return true;
        }
        return false;
    }

    const char *intensity_prefix = "config log intensity ";
    const size_t intensity_prefix_len = strlen(intensity_prefix);
    if (strncmp(normalized, intensity_prefix, intensity_prefix_len) == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SET;
        parsed.key = BLINKY_CONFIG_KEY_LOG_INTENSITY;
        parsed.value.kind = BLINKY_CONFIG_VALUE_BOOL;
        if (parse_bool_token(normalized + intensity_prefix_len, &parsed.value.as.bool_value)) {
            *out = parsed;
            return true;
        }
        return false;
    }

    const char *level_prefix = "config log level ";
    const size_t level_prefix_len = strlen(level_prefix);
    if (strncmp(normalized, level_prefix, level_prefix_len) == 0) {
        parsed.action = BLINKY_CONFIG_CMD_SET;
        parsed.key = BLINKY_CONFIG_KEY_LOG_LEVEL;
        parsed.value.kind = BLINKY_CONFIG_VALUE_LOG_LEVEL;
        if (parse_log_level_token(normalized + level_prefix_len, &parsed.value.as.log_level)) {
            *out = parsed;
            return true;
        }
        return false;
    }

    return false;
}
