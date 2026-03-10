#include "unity.h"

#include "button_policy.h"

TEST_CASE("button policy keeps explicit timing values", "[button_policy]")
{
    button_policy_timing_t in = {
        .debounce_count = 4,
        .long_press_ms = 2500,
    };
    button_policy_timing_t out = button_policy_timing_normalize(in);
    TEST_ASSERT_EQUAL_UINT8(4, out.debounce_count);
    TEST_ASSERT_EQUAL_UINT32(2500, out.long_press_ms);
}

TEST_CASE("button policy normalizes zero debounce to default", "[button_policy]")
{
    button_policy_timing_t in = {
        .debounce_count = 0,
        .long_press_ms = 1500,
    };
    button_policy_timing_t out = button_policy_timing_normalize(in);
    TEST_ASSERT_EQUAL_UINT8(1, out.debounce_count);
    TEST_ASSERT_EQUAL_UINT32(1500, out.long_press_ms);
}

TEST_CASE("button policy normalizes zero long press to default", "[button_policy]")
{
    button_policy_timing_t in = {
        .debounce_count = 2,
        .long_press_ms = 0,
    };
    button_policy_timing_t out = button_policy_timing_normalize(in);
    TEST_ASSERT_EQUAL_UINT8(2, out.debounce_count);
    TEST_ASSERT_EQUAL_UINT32(3000, out.long_press_ms);
}
