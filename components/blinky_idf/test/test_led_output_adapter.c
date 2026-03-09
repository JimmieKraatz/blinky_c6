#include "unity.h"

#include "led_output_adapter.h"

typedef struct {
    int brightness_calls;
    int percent_calls;
    led_brightness_t last_brightness;
    led_percent_t last_percent;
} fake_adapter_ctx_t;

static void fake_set_brightness(void *ctx, led_brightness_t brightness)
{
    fake_adapter_ctx_t *f = (fake_adapter_ctx_t *)ctx;
    f->brightness_calls++;
    f->last_brightness = brightness;
}

static void fake_set_percent(void *ctx, led_percent_t percent)
{
    fake_adapter_ctx_t *f = (fake_adapter_ctx_t *)ctx;
    f->percent_calls++;
    f->last_percent = percent;
}

TEST_CASE("adapter dispatches brightness calls to ops", "[led_output_adapter]")
{
    fake_adapter_ctx_t fake = {0};
    led_output_adapter_t adapter = {
        .ops = &(led_output_adapter_ops_t){
            .set_brightness = fake_set_brightness,
            .set_percent = fake_set_percent,
        },
        .ctx = &fake,
    };

    led_output_adapter_set_brightness(&adapter, 1234);

    TEST_ASSERT_EQUAL_INT(1, fake.brightness_calls);
    TEST_ASSERT_EQUAL_UINT16(1234, fake.last_brightness);
    TEST_ASSERT_EQUAL_INT(0, fake.percent_calls);
}

TEST_CASE("adapter write maps bool to percent", "[led_output_adapter]")
{
    fake_adapter_ctx_t fake = {0};
    led_output_adapter_t adapter = {
        .ops = &(led_output_adapter_ops_t){
            .set_brightness = fake_set_brightness,
            .set_percent = fake_set_percent,
        },
        .ctx = &fake,
    };

    led_output_adapter_write(&adapter, true);
    TEST_ASSERT_EQUAL_INT(1, fake.percent_calls);
    TEST_ASSERT_EQUAL_UINT8(100, fake.last_percent);

    led_output_adapter_write(&adapter, false);
    TEST_ASSERT_EQUAL_INT(2, fake.percent_calls);
    TEST_ASSERT_EQUAL_UINT8(0, fake.last_percent);
}
