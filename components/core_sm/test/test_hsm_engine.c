#include "unity.h"

#include "hsm_engine.h"

typedef enum {
    ST_ROOT = 0,
    ST_ACTIVE,
    ST_RUNNING,
    ST_PAUSED,
    ST_MENU,
    ST_COUNT
} test_state_t;

typedef enum {
    EV_TOGGLE = 1,
    EV_OPEN_MENU,
    EV_BACK
} test_signal_t;

typedef struct {
    hsm_state_t state;
    int enter_root;
    int enter_active;
    int enter_running;
    int enter_paused;
    int enter_menu;
    int exit_root;
    int exit_active;
    int exit_running;
    int exit_paused;
    int exit_menu;
} test_ctx_t;

static void enter_root(void *ctx)    { ((test_ctx_t *)ctx)->enter_root++; }
static void enter_active(void *ctx)  { ((test_ctx_t *)ctx)->enter_active++; }
static void enter_running(void *ctx) { ((test_ctx_t *)ctx)->enter_running++; }
static void enter_paused(void *ctx)  { ((test_ctx_t *)ctx)->enter_paused++; }
static void enter_menu(void *ctx)    { ((test_ctx_t *)ctx)->enter_menu++; }

static void exit_root(void *ctx)    { ((test_ctx_t *)ctx)->exit_root++; }
static void exit_active(void *ctx)  { ((test_ctx_t *)ctx)->exit_active++; }
static void exit_running(void *ctx) { ((test_ctx_t *)ctx)->exit_running++; }
static void exit_paused(void *ctx)  { ((test_ctx_t *)ctx)->exit_paused++; }
static void exit_menu(void *ctx)    { ((test_ctx_t *)ctx)->exit_menu++; }

static hsm_dispatch_result_t dispatch_root(void *ctx, const hsm_event_t *ev, hsm_state_t *target)
{
    (void)ctx;
    (void)ev;
    (void)target;
    return HSM_DISPATCH_IGNORED;
}

static hsm_dispatch_result_t dispatch_active(void *ctx, const hsm_event_t *ev, hsm_state_t *target)
{
    test_ctx_t *c = (test_ctx_t *)ctx;
    if (ev->signal == EV_TOGGLE) {
        *target = (c->state == ST_RUNNING) ? ST_PAUSED : ST_RUNNING;
        return HSM_DISPATCH_TRANSITION;
    }
    (void)target;
    return HSM_DISPATCH_IGNORED;
}

static hsm_dispatch_result_t dispatch_running(void *ctx, const hsm_event_t *ev, hsm_state_t *target)
{
    (void)ctx;
    if (ev->signal == EV_OPEN_MENU) {
        *target = ST_MENU;
        return HSM_DISPATCH_TRANSITION;
    }
    (void)target;
    return HSM_DISPATCH_IGNORED;
}

static hsm_dispatch_result_t dispatch_paused(void *ctx, const hsm_event_t *ev, hsm_state_t *target)
{
    (void)ctx;
    (void)ev;
    (void)target;
    return HSM_DISPATCH_IGNORED;
}

static hsm_dispatch_result_t dispatch_menu(void *ctx, const hsm_event_t *ev, hsm_state_t *target)
{
    (void)ctx;
    if (ev->signal == EV_BACK) {
        *target = ST_RUNNING;
        return HSM_DISPATCH_TRANSITION;
    }
    (void)target;
    return HSM_DISPATCH_IGNORED;
}

static const hsm_state_def_t STATE[ST_COUNT] = {
    [ST_ROOT] = {
        .parent = HSM_STATE_NONE,
        .enter = enter_root,
        .exit = exit_root,
        .dispatch = dispatch_root,
    },
    [ST_ACTIVE] = {
        .parent = ST_ROOT,
        .enter = enter_active,
        .exit = exit_active,
        .dispatch = dispatch_active,
    },
    [ST_RUNNING] = {
        .parent = ST_ACTIVE,
        .enter = enter_running,
        .exit = exit_running,
        .dispatch = dispatch_running,
    },
    [ST_PAUSED] = {
        .parent = ST_ACTIVE,
        .enter = enter_paused,
        .exit = exit_paused,
        .dispatch = dispatch_paused,
    },
    [ST_MENU] = {
        .parent = ST_ROOT,
        .enter = enter_menu,
        .exit = exit_menu,
        .dispatch = dispatch_menu,
    },
};

TEST_CASE("hsm_enter_initial enters root to leaf", "[hsm]")
{
    test_ctx_t c = {0};

    hsm_enter_initial(&c, STATE, ST_COUNT, ST_RUNNING);

    TEST_ASSERT_EQUAL(ST_RUNNING, c.state);
    TEST_ASSERT_EQUAL(1, c.enter_root);
    TEST_ASSERT_EQUAL(1, c.enter_active);
    TEST_ASSERT_EQUAL(1, c.enter_running);
    TEST_ASSERT_EQUAL(0, c.enter_paused);
    TEST_ASSERT_EQUAL(0, c.enter_menu);
}

TEST_CASE("hsm dispatch bubbles and transitions between siblings", "[hsm]")
{
    test_ctx_t c = {0};
    hsm_event_t ev = { .signal = EV_TOGGLE, .param = 0 };
    hsm_dispatch_result_t res = HSM_DISPATCH_IGNORED;

    hsm_enter_initial(&c, STATE, ST_COUNT, ST_RUNNING);
    res = hsm_dispatch_event(&c, STATE, ST_COUNT, &ev);

    TEST_ASSERT_EQUAL(HSM_DISPATCH_TRANSITION, res);
    TEST_ASSERT_EQUAL(ST_PAUSED, c.state);
    TEST_ASSERT_EQUAL(1, c.exit_running);
    TEST_ASSERT_EQUAL(0, c.exit_active);
    TEST_ASSERT_EQUAL(1, c.enter_paused);
    TEST_ASSERT_EQUAL(1, c.enter_active);
}

TEST_CASE("hsm transition across branches exits to lca then enters target path", "[hsm]")
{
    test_ctx_t c = {0};
    hsm_event_t ev = { .signal = EV_OPEN_MENU, .param = 0 };
    hsm_dispatch_result_t res = HSM_DISPATCH_IGNORED;

    hsm_enter_initial(&c, STATE, ST_COUNT, ST_RUNNING);
    res = hsm_dispatch_event(&c, STATE, ST_COUNT, &ev);

    TEST_ASSERT_EQUAL(HSM_DISPATCH_TRANSITION, res);
    TEST_ASSERT_EQUAL(ST_MENU, c.state);
    TEST_ASSERT_EQUAL(1, c.exit_running);
    TEST_ASSERT_EQUAL(1, c.exit_active);
    TEST_ASSERT_EQUAL(0, c.exit_root);
    TEST_ASSERT_EQUAL(1, c.enter_menu);
    TEST_ASSERT_EQUAL(1, c.enter_root);
}

TEST_CASE("hsm event queue push pop and dispatch_next", "[hsm]")
{
    test_ctx_t c = {0};
    hsm_event_t buf[2];
    hsm_event_queue_t q;
    hsm_dispatch_result_t res = HSM_DISPATCH_IGNORED;
    bool has_event = false;

    hsm_enter_initial(&c, STATE, ST_COUNT, ST_RUNNING);
    hsm_event_queue_init(&q, buf, 2);

    TEST_ASSERT_TRUE(hsm_event_push(&q, (hsm_event_t){ .signal = EV_OPEN_MENU, .param = 0 }));
    TEST_ASSERT_TRUE(hsm_event_push(&q, (hsm_event_t){ .signal = EV_BACK, .param = 0 }));
    TEST_ASSERT_FALSE(hsm_event_push(&q, (hsm_event_t){ .signal = EV_TOGGLE, .param = 0 }));

    has_event = hsm_dispatch_next(&c, STATE, ST_COUNT, &q, &res);
    TEST_ASSERT_TRUE(has_event);
    TEST_ASSERT_EQUAL(HSM_DISPATCH_TRANSITION, res);
    TEST_ASSERT_EQUAL(ST_MENU, c.state);

    has_event = hsm_dispatch_next(&c, STATE, ST_COUNT, &q, &res);
    TEST_ASSERT_TRUE(has_event);
    TEST_ASSERT_EQUAL(HSM_DISPATCH_TRANSITION, res);
    TEST_ASSERT_EQUAL(ST_RUNNING, c.state);

    has_event = hsm_dispatch_next(&c, STATE, ST_COUNT, &q, &res);
    TEST_ASSERT_FALSE(has_event);
}
