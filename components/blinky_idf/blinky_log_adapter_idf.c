#include "blinky_log_adapter_idf.h"

#include <inttypes.h>
#include <stdio.h>

#include "esp_log.h"

static esp_log_level_t to_esp_level(blinky_log_level_t level)
{
    switch (level) {
    case BLINKY_LOG_LEVEL_ERROR:
        return ESP_LOG_ERROR;
    case BLINKY_LOG_LEVEL_WARN:
        return ESP_LOG_WARN;
    case BLINKY_LOG_LEVEL_DEBUG:
        return ESP_LOG_DEBUG;
    case BLINKY_LOG_LEVEL_INFO:
    default:
        return ESP_LOG_INFO;
    }
}

static size_t append_text(char *buf, size_t cap, size_t used, const char *text)
{
    if (!buf || cap == 0 || used >= cap) {
        return used;
    }
    int wrote = snprintf(buf + used, cap - used, "%s", text ? text : "");
    if (wrote <= 0) {
        return used;
    }
    size_t next = used + (size_t)wrote;
    return next < cap ? next : cap - 1;
}

static size_t append_kv(char *buf, size_t cap, size_t used, const blinky_log_kv_t *kv)
{
    if (!buf || !kv || cap == 0 || used >= cap) {
        return used;
    }

    const char *key = kv->key ? kv->key : "k";
    int wrote = 0;
    switch (kv->type) {
    case BLINKY_LOG_KV_INT:
        wrote = snprintf(buf + used, cap - used, "%s=%" PRId32, key, kv->value.i32);
        break;
    case BLINKY_LOG_KV_UINT:
        wrote = snprintf(buf + used, cap - used, "%s=%" PRIu32, key, kv->value.u32);
        break;
    case BLINKY_LOG_KV_BOOL:
        wrote = snprintf(buf + used, cap - used, "%s=%s", key, kv->value.b ? "true" : "false");
        break;
    case BLINKY_LOG_KV_STR:
    default:
        wrote = snprintf(buf + used, cap - used, "%s=%s", key, kv->value.s ? kv->value.s : "");
        break;
    }
    if (wrote <= 0) {
        return used;
    }
    size_t next = used + (size_t)wrote;
    return next < cap ? next : cap - 1;
}

static void idf_emit(void *ctx, const blinky_log_record_t *record)
{
    blinky_log_adapter_idf_t *adapter = (blinky_log_adapter_idf_t *)ctx;
    if (!adapter || !record || record->level > adapter->min_level) {
        return;
    }

    char line[224] = {0};
    size_t used = 0;
    used = append_text(line, sizeof(line), used, record->domain ? record->domain : "app");
    used = append_text(line, sizeof(line), used, ":");
    used = append_text(line, sizeof(line), used, record->event ? record->event : "event");
    if (record->message && record->message[0] != '\0') {
        used = append_text(line, sizeof(line), used, " ");
        used = append_text(line, sizeof(line), used, record->message);
    }
    for (uint8_t i = 0; i < record->kv_count; ++i) {
        used = append_text(line, sizeof(line), used, i == 0 ? " | " : " ");
        used = append_kv(line, sizeof(line), used, &record->kvs[i]);
    }

    esp_log_write(to_esp_level(record->level), adapter->tag, "%s\n", line);
}

void blinky_log_adapter_idf_init(blinky_log_sink_t *sink,
                                 blinky_log_adapter_idf_t *adapter,
                                 const blinky_log_adapter_idf_config_t *cfg)
{
    if (!sink || !adapter) {
        return;
    }

    static const blinky_log_sink_ops_t ops = {
        .emit = idf_emit,
    };

    adapter->tag = (cfg && cfg->tag) ? cfg->tag : "blinky";
    adapter->min_level = cfg ? cfg->min_level : BLINKY_LOG_LEVEL_INFO;

    sink->ops = &ops;
    sink->ctx = adapter;
}
