#include "unity.h"

#include "fsm_engine.h"

typedef enum {
    ST_A = 0,
    ST_B,
    ST_COUNT
} test_state_t;

typedef struct {
    fsm_state_t state;
    int enters_a;
    int enters_b;
    bool go_b;
} test_ctx_t;

static void enter_a(void *ctx)
{
    test_ctx_t *c = (test_ctx_t *)ctx;
    c->enters_a++;
}

static void enter_b(void *ctx)
{
    test_ctx_t *c = (test_ctx_t *)ctx;
    c->enters_b++;
}

static fsm_state_t next_a(void *ctx)
{
    test_ctx_t *c = (test_ctx_t *)ctx;
    return c->go_b ? ST_B : ST_A;
}

static fsm_state_t next_b(void *ctx)
{
    (void)ctx;
    return ST_B;
}

static const fsm_state_def_t STATE[ST_COUNT] = {
    [ST_A] = { .enter = enter_a, .next = next_a },
    [ST_B] = { .enter = enter_b, .next = next_b },
};

/* Ensure enter hook runs and initial state is recorded. */
TEST_CASE("fsm_enter sets initial state and calls enter", "[fsm]")
{
    test_ctx_t c = {0};

    fsm_enter(&c, STATE, ST_COUNT, ST_A, true);

    TEST_ASSERT_EQUAL(ST_A, c.state);
    TEST_ASSERT_EQUAL(1, c.enters_a);
    TEST_ASSERT_EQUAL(0, c.enters_b);
}

/* Verify transition occurs when next state differs. */
TEST_CASE("fsm_step transitions when next changes", "[fsm]")
{
    test_ctx_t c = {0};

    fsm_enter(&c, STATE, ST_COUNT, ST_A, true);
    c.go_b = true;
    fsm_step(&c, STATE, ST_COUNT, false);

    TEST_ASSERT_EQUAL(ST_B, c.state);
    TEST_ASSERT_EQUAL(1, c.enters_a);
    TEST_ASSERT_EQUAL(1, c.enters_b);
}

/* Default behavior: no reentry when next state is unchanged. */
TEST_CASE("fsm_step does not reenter same state by default", "[fsm]")
{
    test_ctx_t c = {0};

    fsm_enter(&c, STATE, ST_COUNT, ST_A, true);
    fsm_step(&c, STATE, ST_COUNT, false);

    TEST_ASSERT_EQUAL(ST_A, c.state);
    TEST_ASSERT_EQUAL(1, c.enters_a);
}

/* Optional behavior: reenter same state when explicitly enabled. */
TEST_CASE("fsm_step can reenter same state when enabled", "[fsm]")
{
    test_ctx_t c = {0};

    fsm_enter(&c, STATE, ST_COUNT, ST_A, true);
    fsm_step(&c, STATE, ST_COUNT, true);

    TEST_ASSERT_EQUAL(ST_A, c.state);
    TEST_ASSERT_EQUAL(2, c.enters_a);
}
