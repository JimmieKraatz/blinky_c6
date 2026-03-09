#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*
 * Generic FSM engine.
 *
 * Usage pattern:
 *   typedef enum { ST_A, ST_B, ST_COUNT } state_t;
 *
 *   typedef struct {
 *       fsm_state_t state;  // required by engine; must be first member
 *       ... fields ...
 *   } sm_t;
 *
 *   static void enter_a(void *ctx) { ... }
 *   static fsm_state_t next_a(void *ctx) { ... return ST_B; }
 *
 *   static const fsm_state_def_t STATE[ST_COUNT] = {
 *       [ST_A] = { .enter = enter_a, .next = next_a },
 *       ...
 *   };
 *
 *   fsm_enter(&sm, STATE, ST_COUNT, ST_A, false);
 *   fsm_step(&sm, STATE, ST_COUNT, false);
*/

typedef size_t fsm_state_t;

/* State callbacks operate on an opaque context pointer. */
typedef void        (*fsm_enter_fn)(void *ctx);
typedef fsm_state_t (*fsm_next_fn)(void *ctx);

typedef struct {
    fsm_enter_fn enter;
    fsm_next_fn  next;
} fsm_state_def_t;

/*
 * Enter a state.
 *
 * - ctx: state machine context struct pointer
 * - states: table of state handlers
 * - count: number of states in table
 * - s: state index to enter
 * - reenter: if false, entering the same state is a no-op; if true enter() runs
 */
static inline void fsm_enter(void * ctx,
                         const fsm_state_def_t * states,
                         size_t count,
                         fsm_state_t s,
                         bool reenter)
{
    assert(states);
    assert(s < count);
    assert(states[s].enter && states[s].next);

    fsm_state_t *cur = (fsm_state_t *)ctx; /* assumes state is first member */

    if (!reenter && *cur == s) {
        return;
    }

    *cur = s;
    states[s].enter(ctx);
}

/*
 * Step the FSM once.
 *
 * - Calls next(ctx) for current state and enters the returned state if different
 * - If reenter_on_same is true, enter() is called even when next == current
 */
static inline void fsm_step(void *ctx,
                        const fsm_state_def_t *states,
                        size_t count,
                        bool reenter_on_same)
{
    fsm_state_t *cur = (fsm_state_t *)ctx; /* assumes state is first member */
    
    assert(states);
    assert(*cur < count);
    assert(states[*cur].enter && states[*cur].next);
    
    fsm_state_t nxt = states[*cur].next(ctx);

    if (nxt != *cur || reenter_on_same) {
        fsm_enter(ctx, states, count, nxt, true);
    }
}
