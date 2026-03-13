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
