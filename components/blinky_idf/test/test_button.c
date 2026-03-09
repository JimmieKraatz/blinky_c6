#include "unity.h"

#include "button.h"

static button_t make_button(uint32_t long_press_ms)
{
    button_t btn = {0};
    button_init(&btn, GPIO_NUM_9, true, BUTTON_PULL_NONE, 3, long_press_ms);
    return btn;
}

static button_event_t feed(button_t *btn, bool raw_pressed, TickType_t now)
{
    return button_poll_event_raw(btn, raw_pressed, now);
}

/* Verify debounce and release path yields a short-press event. */
TEST_CASE("button short press requires debounce then release", "[button]")
{
    button_t btn = make_button(3000);
    TickType_t t = 0;

    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, false, t++));

    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, true, t++));
    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, true, t++));
    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, true, t++));

    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, true, t++));

    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, false, t++));
    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, false, t++));
    TEST_ASSERT_EQUAL(BUTTON_EVENT_SHORT_PRESS, feed(&btn, false, t++));
}

/* Verify long-press fires once and suppresses short-press on release. */
TEST_CASE("button long press fires once and suppresses short press", "[button]")
{
    button_t btn = make_button(10);
    TickType_t t = 0;

    feed(&btn, false, t++);
    feed(&btn, true, t++);
    feed(&btn, true, t++);
    feed(&btn, true, t++);

    TEST_ASSERT_EQUAL(BUTTON_EVENT_LONG_PRESS, feed(&btn, true, t + 10));
    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, true, t + 11));

    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, false, t + 12));
    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, false, t + 13));
    TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, feed(&btn, false, t + 14));
}
