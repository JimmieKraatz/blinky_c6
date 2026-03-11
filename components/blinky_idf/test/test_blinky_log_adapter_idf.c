#include "blinky_log_adapter_idf.h"

#include "unity.h"

TEST_CASE("idf log adapter init wires sink and defaults", "[blinky_log_adapter_idf]")
{
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
    blinky_log_sink_t sink = {0};
    blinky_log_adapter_idf_t adapter = {0};
    blinky_log_adapter_idf_init(&sink, &adapter, NULL);

    blinky_log_emit(&sink, NULL);
    blinky_log_emit(NULL, NULL);

    TEST_ASSERT_TRUE(true);
}
