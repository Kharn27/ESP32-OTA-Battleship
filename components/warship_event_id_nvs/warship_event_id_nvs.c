#include "warship_event_id_nvs.h"
#include "nvs.h"

#define ID_NVS_NAMESPACE "warship"
#define ID_NVS_KEY "event_seq"
#define ID_MAGIC UINT32_C(0x57494431)
#define ID_VERSION UINT32_C(1)
#define ID_BYTES 20u

static void put32(uint8_t *p, uint32_t value)
{
    for (unsigned i=0u; i<4u; ++i)
        p[i] = (uint8_t)(value >> (8u*i));
}
static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint32_t fnv16(const uint8_t *p)
{
    uint32_t h = UINT32_C(2166136261);
    for (unsigned i=0u;i<16u;++i) {
        h ^= p[i];
        h *= UINT32_C(16777619);
    }
    return h;
}
static const char *ns(const WarshipEventIdNvsContext *c)
{
    return c->namespace_name != NULL ? c->namespace_name : ID_NVS_NAMESPACE;
}
static int load(void *ctx, uint64_t *out)
{
    WarshipEventIdNvsContext *c = (WarshipEventIdNvsContext *)ctx;
    if (c == NULL || out == NULL) return -1;
    nvs_handle_t handle;
    esp_err_t err = nvs_open(ns(c),NVS_READONLY,&handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        c->last_error = ESP_OK;
        return 1;
    }
    if (err != ESP_OK) {
        c->last_error = err;
        return -1;
    }
    size_t length = 0u;
    err = nvs_get_blob(handle,ID_NVS_KEY,NULL,&length);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        c->last_error = ESP_OK;
        return 1;
    }
    if (err != ESP_OK) {
        nvs_close(handle);
        c->last_error = err;
        return -1;
    }
    if (length != ID_BYTES) {
        nvs_close(handle);
        c->last_error = ESP_OK;
        return -2;
    }
    uint8_t bytes[ID_BYTES];
    err = nvs_get_blob(handle,ID_NVS_KEY,bytes,&length);
    nvs_close(handle);
    if (err != ESP_OK) {
        c->last_error = err;
        return -1;
    }
    c->last_error = ESP_OK;
    if (length != ID_BYTES || get32(bytes) != ID_MAGIC ||
        get32(bytes+16u) != fnv16(bytes)) return -2;
    if (get32(bytes+4u) != ID_VERSION) return -3;
    const uint64_t id = (uint64_t)get32(bytes+8u) |
        ((uint64_t)get32(bytes+12u) << 32u);
    if (id == 0u) return -2;
    *out = id;
    return 0;
}
static int reserve(void *ctx, uint64_t id)
{
    WarshipEventIdNvsContext *c = (WarshipEventIdNvsContext *)ctx;
    if (c == NULL || id == 0u) return -1;
    uint8_t bytes[ID_BYTES];
    put32(bytes,ID_MAGIC);
    put32(bytes+4u,ID_VERSION);
    put32(bytes+8u,(uint32_t)id);
    put32(bytes+12u,(uint32_t)(id >> 32u));
    put32(bytes+16u,fnv16(bytes));
    nvs_handle_t handle;
    esp_err_t err = nvs_open(ns(c),NVS_READWRITE,&handle);
    if (err != ESP_OK) {
        c->last_error = err;
        return -1;
    }
    err = nvs_set_blob(handle,ID_NVS_KEY,bytes,sizeof bytes);
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    c->last_error = err;
    return err == ESP_OK ? 0 : -1;
}
WarshipEventIdBackend warship_event_id_nvs_backend(void)
{
    WarshipEventIdBackend b = {load,reserve};
    return b;
}
