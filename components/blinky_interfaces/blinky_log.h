#ifndef BLINKY_LOG_H
#define BLINKY_LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    BLINKY_LOG_LEVEL_ERROR = 0,
    BLINKY_LOG_LEVEL_WARN = 1,
    BLINKY_LOG_LEVEL_INFO = 2,
    BLINKY_LOG_LEVEL_DEBUG = 3,
} blinky_log_level_t;

typedef enum {
    BLINKY_LOG_KV_INT = 0,
    BLINKY_LOG_KV_UINT = 1,
    BLINKY_LOG_KV_BOOL = 2,
    BLINKY_LOG_KV_STR = 3,
} blinky_log_kv_type_t;

typedef struct {
    const char *key;
    blinky_log_kv_type_t type;
    union {
        int32_t i32;
        uint32_t u32;
        bool b;
        const char *s;
    } value;
} blinky_log_kv_t;

typedef struct {
    blinky_log_level_t level;
    const char *domain;
    const char *event;
    const char *message;
    const blinky_log_kv_t *kvs;
    uint8_t kv_count;
} blinky_log_record_t;

typedef void (*blinky_log_emit_fn)(void *ctx, const blinky_log_record_t *record);

typedef struct {
    blinky_log_emit_fn emit;
} blinky_log_sink_ops_t;

typedef struct {
    const blinky_log_sink_ops_t *ops;
    void *ctx;
} blinky_log_sink_t;

static inline blinky_log_kv_t blinky_log_kv_int(const char *key, int32_t value)
{
    blinky_log_kv_t kv = {
        .key = key,
        .type = BLINKY_LOG_KV_INT,
        .value.i32 = value,
    };
    return kv;
}

static inline blinky_log_kv_t blinky_log_kv_uint(const char *key, uint32_t value)
{
    blinky_log_kv_t kv = {
        .key = key,
        .type = BLINKY_LOG_KV_UINT,
        .value.u32 = value,
    };
    return kv;
}

static inline blinky_log_kv_t blinky_log_kv_bool(const char *key, bool value)
{
    blinky_log_kv_t kv = {
        .key = key,
        .type = BLINKY_LOG_KV_BOOL,
        .value.b = value,
    };
    return kv;
}

static inline blinky_log_kv_t blinky_log_kv_str(const char *key, const char *value)
{
    blinky_log_kv_t kv = {
        .key = key,
        .type = BLINKY_LOG_KV_STR,
        .value.s = value,
    };
    return kv;
}

static inline void blinky_log_emit(blinky_log_sink_t *sink, const blinky_log_record_t *record)
{
    if (sink && sink->ops && sink->ops->emit) {
        sink->ops->emit(sink->ctx, record);
    }
}

#endif
