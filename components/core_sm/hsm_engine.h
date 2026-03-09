#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Generic event-driven hierarchical state machine (HSM) engine.
 *
 * Design goals:
 * - Keep the same context pattern as fsm_engine: current state must be first member.
 * - Support parent-chain event bubbling.
 * - Support explicit transitions with correct exit/enter sequencing via LCA.
 * - Provide an optional fixed-size event queue for step-wise event-driven execution.
 */

typedef size_t hsm_state_t;

#define HSM_STATE_NONE ((hsm_state_t)SIZE_MAX)

#ifndef HSM_MAX_DEPTH
#define HSM_MAX_DEPTH 16U
#endif

typedef struct {
    uint16_t signal;
    uintptr_t param;
} hsm_event_t;

typedef enum {
    HSM_DISPATCH_IGNORED = 0,
    HSM_DISPATCH_HANDLED,
    HSM_DISPATCH_TRANSITION
} hsm_dispatch_result_t;

typedef void (*hsm_enter_fn)(void *ctx);
typedef void (*hsm_exit_fn)(void *ctx);
typedef hsm_dispatch_result_t (*hsm_dispatch_fn)(void *ctx, const hsm_event_t *ev, hsm_state_t *target);

typedef struct {
    hsm_state_t parent;
    hsm_enter_fn enter;
    hsm_exit_fn exit;
    hsm_dispatch_fn dispatch;
} hsm_state_def_t;

typedef struct {
    hsm_event_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
} hsm_event_queue_t;

static inline size_t hsm_build_ancestry(const hsm_state_def_t *states,
                                        size_t count,
                                        hsm_state_t leaf,
                                        hsm_state_t *path,
                                        size_t path_cap)
{
    size_t depth = 0;
    hsm_state_t s = leaf;

    while (s != HSM_STATE_NONE) {
        assert(s < count);
        assert(depth < path_cap);
        path[depth++] = s;
        s = states[s].parent;
    }
    return depth;
}

static inline void hsm_transition(void *ctx,
                                  const hsm_state_def_t *states,
                                  size_t count,
                                  hsm_state_t target)
{
    hsm_state_t *cur = (hsm_state_t *)ctx; /* assumes state is first member */
    hsm_state_t cur_path[HSM_MAX_DEPTH];
    hsm_state_t target_path[HSM_MAX_DEPTH];
    size_t cur_depth = 0;
    size_t target_depth = 0;
    size_t i = 0;
    size_t j = 0;

    assert(states);
    assert(target < count);
    assert(count <= HSM_MAX_DEPTH);

    if (*cur == target) {
        return;
    }

    if (*cur != HSM_STATE_NONE) {
        cur_depth = hsm_build_ancestry(states, count, *cur, cur_path, HSM_MAX_DEPTH);
    }
    target_depth = hsm_build_ancestry(states, count, target, target_path, HSM_MAX_DEPTH);

    i = cur_depth;
    j = target_depth;
    while (i > 0 && j > 0 && cur_path[i - 1] == target_path[j - 1]) {
        --i;
        --j;
    }

    for (size_t k = 0; k < i; ++k) {
        hsm_exit_fn on_exit = states[cur_path[k]].exit;
        if (on_exit) {
            on_exit(ctx);
        }
    }
    for (size_t k = j; k > 0; --k) {
        hsm_enter_fn on_enter = states[target_path[k - 1]].enter;
        if (on_enter) {
            on_enter(ctx);
        }
    }

    *cur = target;
}

/* Enter initial state: runs enter hooks from root -> leaf. */
static inline void hsm_enter_initial(void *ctx,
                                     const hsm_state_def_t *states,
                                     size_t count,
                                     hsm_state_t initial)
{
    hsm_state_t *cur = (hsm_state_t *)ctx; /* assumes state is first member */

    assert(ctx);
    assert(states);
    assert(count > 0);
    assert(initial < count);

    *cur = HSM_STATE_NONE;
    hsm_transition(ctx, states, count, initial);
}

/* Dispatch one event with bubbling from current leaf to root. */
static inline hsm_dispatch_result_t hsm_dispatch_event(void *ctx,
                                                       const hsm_state_def_t *states,
                                                       size_t count,
                                                       const hsm_event_t *ev)
{
    hsm_state_t *cur = (hsm_state_t *)ctx; /* assumes state is first member */
    hsm_state_t s = 0;

    assert(ctx);
    assert(states);
    assert(ev);
    assert(*cur < count);

    s = *cur;
    while (s != HSM_STATE_NONE) {
        const hsm_state_def_t *def = &states[s];
        if (def->dispatch) {
            hsm_state_t target = HSM_STATE_NONE;
            hsm_dispatch_result_t res = def->dispatch(ctx, ev, &target);
            if (res == HSM_DISPATCH_HANDLED) {
                return HSM_DISPATCH_HANDLED;
            }
            if (res == HSM_DISPATCH_TRANSITION) {
                assert(target < count);
                hsm_transition(ctx, states, count, target);
                return HSM_DISPATCH_TRANSITION;
            }
        }
        s = def->parent;
    }

    return HSM_DISPATCH_IGNORED;
}

static inline void hsm_event_queue_init(hsm_event_queue_t *q, hsm_event_t *buffer, size_t capacity)
{
    assert(q);
    assert(buffer);
    assert(capacity > 0);

    q->buffer = buffer;
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

static inline bool hsm_event_push(hsm_event_queue_t *q, hsm_event_t ev)
{
    assert(q);
    assert(q->buffer);
    assert(q->capacity > 0);

    if (q->count == q->capacity) {
        return false;
    }
    q->buffer[q->tail] = ev;
    q->tail = (q->tail + 1U) % q->capacity;
    q->count++;
    return true;
}

static inline bool hsm_event_pop(hsm_event_queue_t *q, hsm_event_t *out_ev)
{
    assert(q);
    assert(q->buffer);
    assert(q->capacity > 0);
    assert(out_ev);

    if (q->count == 0) {
        return false;
    }
    *out_ev = q->buffer[q->head];
    q->head = (q->head + 1U) % q->capacity;
    q->count--;
    return true;
}

/*
 * Pop and dispatch one event.
 * Returns false when the queue is empty.
 */
static inline bool hsm_dispatch_next(void *ctx,
                                     const hsm_state_def_t *states,
                                     size_t count,
                                     hsm_event_queue_t *q,
                                     hsm_dispatch_result_t *out_res)
{
    hsm_event_t ev = {0};
    hsm_dispatch_result_t res = HSM_DISPATCH_IGNORED;

    assert(q);

    if (!hsm_event_pop(q, &ev)) {
        return false;
    }
    res = hsm_dispatch_event(ctx, states, count, &ev);
    if (out_res) {
        *out_res = res;
    }
    return true;
}
