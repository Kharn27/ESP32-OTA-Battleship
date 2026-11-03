#include "warship_palmares_nvs.h"

#include "nvs.h"
#include "nvs_flash.h"

#define NVS_NAMESPACE "warship"
#define NVS_KEY "palmares"

esp_err_t warship_palmares_nvs_init(void)
{
    /* Destructive 'erase and retry' is explicitly forbidden: doing so
     * would discard an existing palmares after a recoverable error. */
    return nvs_flash_init();
}

static int nvs_load(void *ctx, uint8_t *data, size_t capacity, size_t *actual)
{
    WarshipPalmaresNvsContext *c = (WarshipPalmaresNvsContext *)ctx;
    if (c == NULL || data == NULL || actual == NULL) return -1;
    *actual = 0u;
    nvs_handle_t handle;
    esp_err_t err = nvs_open(c->namespace_name != NULL ? c->namespace_name : NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        c->last_error = ESP_OK;
        return 1;
    }
    if (err != ESP_OK) {
        c->last_error = err;
        return -1;
    }

    size_t length = 0u;
    err = nvs_get_blob(handle, NVS_KEY, NULL, &length);
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
    *actual = length;
    if (length > capacity) {
        /* Core classifies this as invalid/corrupt data without writing. */
        nvs_close(handle);
        c->last_error = ESP_OK;
        return 0;
    }
    err = nvs_get_blob(handle, NVS_KEY, data, &length);
    nvs_close(handle);
    if (err != ESP_OK) {
        c->last_error = err;
        return -1;
    }
    *actual = length;
    c->last_error = ESP_OK;
    return 0;
}

static int nvs_store(void *ctx, const uint8_t *data, size_t size)
{
    WarshipPalmaresNvsContext *c = (WarshipPalmaresNvsContext *)ctx;
    if (c == NULL || data == NULL || size != WARSHIP_PALMARES_WIRE_SIZE)
        return -1;
    nvs_handle_t handle;
    esp_err_t err = nvs_open(c->namespace_name != NULL ? c->namespace_name : NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        c->last_error = err;
        return -1;
    }
    err = nvs_set_blob(handle, NVS_KEY, data, size);
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    c->last_error = err;
    return err == ESP_OK ? 0 : -1;
}

WarshipPalmaresBackend warship_palmares_nvs_backend(void)
{
    WarshipPalmaresBackend backend = {nvs_load, nvs_store};
    return backend;
}
