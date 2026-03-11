#include "blinky_log_adapter_idf.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "unity.h"

static int g_log_write_calls;
static esp_log_level_t g_last_level;
static char g_last_tag[32];
static char g_last_line[256];

void blinky_log_adapter_idf_write(esp_log_level_t level, const char *tag, const char *line)
{
    g_log_write_calls += 1;
    g_last_level = level;
    snprintf(g_last_tag, sizeof(g_last_tag), "%s", tag ? tag : "");
    snprintf(g_last_line, sizeof(g_last_line), "%s", line ? line : "");
}

static void reset_log_capture(void)
{
    g_log_write_calls = 0;
    g_last_level = ESP_LOG_NONE;
    g_last_tag[0] = '\0';
    g_last_line[0] = '\0';
}

TEST_CASE("idf log adapter init wires sink and defaults", "[blinky_log_adapter_idf]")
{
    reset_log_capture();
    blinky_log_sink_t sink = {0};
    blinky_log_adapter_idf_t adapter = {0};

    blinky_log_adapter_idf_init(&sink, &adapter, NULL);

    TEST_ASSERT_NOT_NULL(sink.ops);
    TEST_ASSERT_NOT_NULL(sink.ops->emit);
    TEST_ASSERT_EQUAL_PTR(&adapter, sink.ctx);
    TEST_ASSERT_EQUAL_STRING("blinky", adapter.tag);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_INFO, (uint32_t)adapter.min_level);
}

TEST_CASE("idf log adapter init applies configured tag and level", "[blinky_log_adapter_idf]")
{
    reset_log_capture();
    blinky_log_sink_t sink = {0};
    blinky_log_adapter_idf_t adapter = {0};
    const blinky_log_adapter_idf_config_t cfg = {
        .tag = "utest",
        .min_level = BLINKY_LOG_LEVEL_DEBUG,
    };

    blinky_log_adapter_idf_init(&sink, &adapter, &cfg);

    TEST_ASSERT_EQUAL_STRING("utest", adapter.tag);
    TEST_ASSERT_EQUAL_UINT32(BLINKY_LOG_LEVEL_DEBUG, (uint32_t)adapter.min_level);
}

TEST_CASE("idf log adapter emit is null-safe", "[blinky_log_adapter_idf]")
{
    reset_log_capture();
    blinky_log_sink_t sink = {0};
    blinky_log_adapter_idf_t adapter = {0};
    blinky_log_adapter_idf_init(&sink, &adapter, NULL);

    blinky_log_emit(&sink, NULL);
    blinky_log_emit(NULL, NULL);

    TEST_ASSERT_EQUAL_INT(0, g_log_write_calls);
}

TEST_CASE("idf log adapter filters records below min level", "[blinky_log_adapter_idf]")
{
    reset_log_capture();
    blinky_log_sink_t sink = {0};
    blinky_log_adapter_idf_t adapter = {0};
    const blinky_log_adapter_idf_config_t cfg = {
        .tag = "utest",
        .min_level = BLINKY_LOG_LEVEL_WARN,
    };
    const blinky_log_record_t info_record = {
        .level = BLINKY_LOG_LEVEL_INFO,
        .domain = "app",
        .event = "boot",
        .message = "startup",
    };
    const blinky_log_record_t err_record = {
        .level = BLINKY_LOG_LEVEL_ERROR,
        .domain = "app",
        .event = "fault",
        .message = "x",
    };

    blinky_log_adapter_idf_init(&sink, &adapter, &cfg);
    blinky_log_emit(&sink, &info_record);
    TEST_ASSERT_EQUAL_INT(0, g_log_write_calls);

    blinky_log_emit(&sink, &err_record);
    TEST_ASSERT_EQUAL_INT(1, g_log_write_calls);
    TEST_ASSERT_EQUAL(ESP_LOG_ERROR, g_last_level);
    TEST_ASSERT_EQUAL_STRING("utest", g_last_tag);
}

TEST_CASE("idf log adapter formats domain event message and kvs", "[blinky_log_adapter_idf]")
{
    reset_log_capture();
    blinky_log_sink_t sink = {0};
    blinky_log_adapter_idf_t adapter = {0};
    const blinky_log_kv_t kvs[] = {
        blinky_log_kv_int("i", -7),
        blinky_log_kv_uint("u", 9),
        blinky_log_kv_bool("b", true),
        blinky_log_kv_str("s", "ok"),
    };
    const blinky_log_record_t record = {
        .level = BLINKY_LOG_LEVEL_DEBUG,
        .domain = "runtime",
        .event = "state_change",
        .message = "running",
        .kvs = kvs,
        .kv_count = 4,
    };
    const blinky_log_adapter_idf_config_t cfg = {
        .tag = "fmt",
        .min_level = BLINKY_LOG_LEVEL_DEBUG,
    };

    blinky_log_adapter_idf_init(&sink, &adapter, &cfg);
    blinky_log_emit(&sink, &record);

    TEST_ASSERT_EQUAL_INT(1, g_log_write_calls);
    TEST_ASSERT_EQUAL(ESP_LOG_DEBUG, g_last_level);
    TEST_ASSERT_EQUAL_STRING("fmt", g_last_tag);
    TEST_ASSERT_EQUAL_STRING(
        "runtime:state_change running | i=-7 u=9 b=true s=ok", g_last_line);
}
