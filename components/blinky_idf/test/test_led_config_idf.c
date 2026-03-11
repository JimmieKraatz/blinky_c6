#include "unity.h"

#include "led_config_idf.h"

TEST_CASE("producer poll clamp enforces min max and pass-through", "[led_config_idf]")
{
    TEST_ASSERT_EQUAL_UINT32(1, led_config_idf_clamp_producer_poll_ms(0));
    TEST_ASSERT_EQUAL_UINT32(1, led_config_idf_clamp_producer_poll_ms(-10));
    TEST_ASSERT_EQUAL_UINT32(10, led_config_idf_clamp_producer_poll_ms(10));
    TEST_ASSERT_EQUAL_UINT32(1000, led_config_idf_clamp_producer_poll_ms(1001));
}

TEST_CASE("boot pattern clamp enforces min max and pass-through", "[led_config_idf]")
{
    TEST_ASSERT_EQUAL_INT(1, led_config_idf_clamp_boot_pattern_ms(0));
    TEST_ASSERT_EQUAL_INT(1, led_config_idf_clamp_boot_pattern_ms(-5));
    TEST_ASSERT_EQUAL_INT(120, led_config_idf_clamp_boot_pattern_ms(120));
    TEST_ASSERT_EQUAL_INT(2000, led_config_idf_clamp_boot_pattern_ms(5000));
}
