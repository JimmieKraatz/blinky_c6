#include "unity.h"

#include "button_logic.h"

static button_logic_t make_button(uint32_t long_press_ms)
{
    button_logic_t btn = {0};
    button_logic_init(&btn, 3, long_press_ms);
    return btn;
}

static button_logic_event_t feed(button_logic_t *btn, bool raw_pressed, blinky_time_ms_t now)
{
    return button_logic_poll_raw(btn, raw_pressed, now);
}

TEST_CASE("button logic short press requires debounce then release", "[button_logic]")
{
    button_logic_t btn = make_button(3000);
    blinky_time_ms_t t = 0;

    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, false, t++));

    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, true, t++));
    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, true, t++));
    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, true, t++));

    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, true, t++));

    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, false, t++));
    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, false, t++));
    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_SHORT_PRESS, feed(&btn, false, t++));
}

TEST_CASE("button logic long press fires once and suppresses short press", "[button_logic]")
{
    button_logic_t btn = make_button(10);
    blinky_time_ms_t t = 0;

    feed(&btn, false, t++);
    feed(&btn, true, t++);
    feed(&btn, true, t++);
    feed(&btn, true, t++);

    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_LONG_PRESS, feed(&btn, true, t + 10));
    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, true, t + 11));

    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, false, t + 12));
    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, false, t + 13));
    TEST_ASSERT_EQUAL(BUTTON_LOGIC_EVENT_NONE, feed(&btn, false, t + 14));
}
