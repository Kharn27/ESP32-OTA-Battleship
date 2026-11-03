#include "warship_palmares_nvs.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
    exit(EXIT_FAILURE); \
} } while (0)
#define EQ(a,b) CHECK((a) == (b))
#define STATUS(expr,expected) EQ((expr),(expected))
#define APPLIED(expr) STATUS((expr),WARSHIP_PALMARES_APPLIED)

static struct {
    uint8_t blob[WARSHIP_PALMARES_WIRE_SIZE + 2u];
    size_t size;
    int exists;
    uint8_t staged[WARSHIP_PALMARES_WIRE_SIZE];
    size_t staged_size;
    int staged_valid;
    esp_err_t init_error, open_error, get_error, set_error, commit_error;
    int commit_after_write;
    unsigned inits, opens, gets, sets, commits, closes;
} fake;

static void reset_fake(void)
{
    memset(&fake, 0, sizeof fake);
}

esp_err_t nvs_flash_init(void)
{
    ++fake.inits;
    return fake.init_error;
}
esp_err_t nvs_open(const char *name, nvs_open_mode_t mode, nvs_handle_t *out)
{
    ++fake.opens;
    CHECK(strcmp(name, "warship") == 0);
    CHECK(out != NULL);
    if (fake.open_error != ESP_OK) return fake.open_error;
    if (!fake.exists && mode == NVS_READONLY)
        return ESP_ERR_NVS_NOT_FOUND;
    *out = 1u;
    return ESP_OK;
}
esp_err_t nvs_get_blob(nvs_handle_t handle, const char *key,
                       void *data, size_t *length)
{
    ++fake.gets;
    EQ(handle, 1u);
    CHECK(strcmp(key, "palmares") == 0);
    CHECK(length != NULL);
    if (fake.get_error != ESP_OK) return fake.get_error;
    if (!fake.exists) return ESP_ERR_NVS_NOT_FOUND;
    if (data != NULL) {
        CHECK(*length >= fake.size);
        memcpy(data, fake.blob, fake.size);
    }
    *length = fake.size;
    return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t handle, const char *key,
                       const void *data, size_t length)
{
    ++fake.sets;
    EQ(handle, 1u);
    CHECK(strcmp(key, "palmares") == 0);
    if (fake.set_error != ESP_OK) return fake.set_error;
    EQ(length, WARSHIP_PALMARES_WIRE_SIZE);
    memcpy(fake.staged, data, length);
    fake.staged_size = length;
    fake.staged_valid = 1;
    return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle)
{
    ++fake.commits;
    EQ(handle, 1u);
    if (fake.commit_error != ESP_OK && !fake.commit_after_write)
        return fake.commit_error;
    if (fake.staged_valid) {
        memcpy(fake.blob, fake.staged, fake.staged_size);
        fake.size = fake.staged_size;
        fake.exists = 1;
    }
    return fake.commit_error;
}
void nvs_close(nvs_handle_t handle)
{
    ++fake.closes;
    EQ(handle, 1u);
    fake.staged_valid = 0;
}

static WarshipPalmares open_nvs(WarshipPalmaresNvsContext *ctx)
{
    const WarshipPalmaresBackend b = warship_palmares_nvs_backend();
    CHECK(b.load != NULL && b.store != NULL);
    WarshipPalmares ledger;
    memset(&ledger, 0x7f, sizeof ledger);
    APPLIED(warship_palmares_open(&ledger, b, ctx));
    EQ(ctx->last_error, ESP_OK);
    return ledger;
}
static WarshipPalmaresView stats(WarshipPalmares *p)
{
    WarshipPalmaresView v = {0};
    APPLIED(warship_palmares_read(p, &v));
    EQ((uint64_t)v.victories + v.defeats + v.draws, v.games);
    return v;
}

static void test_lifecycle(void)
{
    reset_fake();
    WarshipPalmaresNvsContext ctx = {.last_error = ESP_FAIL};
    fake.init_error = ESP_ERR_NVS_NO_FREE_PAGES;
    EQ(warship_palmares_nvs_init(), ESP_ERR_NVS_NO_FREE_PAGES);
    fake.init_error = ESP_ERR_NVS_NEW_VERSION_FOUND;
    EQ(warship_palmares_nvs_init(), ESP_ERR_NVS_NEW_VERSION_FOUND);
    EQ(fake.sets, 0u); EQ(fake.commits, 0u); /* no erase */
    fake.init_error = ESP_OK;
    EQ(warship_palmares_nvs_init(), ESP_OK);
    WarshipPalmares p = open_nvs(&ctx);
    EQ(fake.exists, 0);
    EQ(fake.sets, 0u);
    WarshipPalmaresView v = stats(&p);
    EQ(v.games, 0u);
    APPLIED(warship_palmares_apply(&p, 1u,
            WARSHIP_LIFECYCLE_EVENT_VICTORY));
    EQ(fake.sets, 1u); EQ(fake.commits, 1u); EQ(fake.exists, 1);
    EQ(ctx.last_error, ESP_OK);
    WarshipPalmares reopened = open_nvs(&ctx);
    v = stats(&reopened);
    EQ(v.games, 1u); EQ(v.victories, 1u);
    STATUS(warship_palmares_apply(&reopened, 1u,
           WARSHIP_LIFECYCLE_EVENT_VICTORY),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    EQ(fake.commits, 1u);

    /* When nvs_set_blob fails there must be no nvs_commit. */
    fake.set_error = ESP_FAIL;
    STATUS(warship_palmares_apply(&reopened, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_WRITE_UNCERTAIN);
    EQ(ctx.last_error, ESP_FAIL);
    EQ(fake.commits, 1u);
    STATUS(warship_palmares_read(&reopened, &v),
           WARSHIP_PALMARES_RELOAD_REQUIRED);
    fake.set_error = ESP_OK;
    APPLIED(warship_palmares_reload(&reopened));
    v = stats(&reopened);
    EQ(v.games, 1u);

    /* Commit succeeds on persistent fake storage but returns an error.
     * The same transaction on a reopened ledger must deduplicate. */
    fake.commit_after_write = 1;
    fake.commit_error = ESP_FAIL;
    STATUS(warship_palmares_apply(&reopened, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_WRITE_UNCERTAIN);
    EQ(ctx.last_error, ESP_FAIL);
    EQ(fake.commits, 2u);
    fake.commit_error = ESP_OK;
    fake.commit_after_write = 0;
    WarshipPalmares restored = open_nvs(&ctx);
    v = stats(&restored);
    EQ(v.games, 2u); EQ(v.defeats, 1u);
    STATUS(warship_palmares_apply(&restored, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    APPLIED(warship_palmares_reload(&reopened));
    STATUS(warship_palmares_apply(&reopened, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_ALREADY_APPLIED);

    /* Inject read/open failures: no reinitialization, no counter loss. */
    fake.get_error = ESP_FAIL;
    WarshipPalmares target;
    memset(&target, 0x59, sizeof target);
    const WarshipPalmares original = target;
    STATUS(warship_palmares_open(&target,
           warship_palmares_nvs_backend(), &ctx),
           WARSHIP_PALMARES_READ_ERROR);
    EQ(ctx.last_error, ESP_FAIL);
    CHECK(memcmp(&target, &original, sizeof target) == 0);
    fake.get_error = ESP_OK;
    fake.open_error = ESP_FAIL;
    STATUS(warship_palmares_open(&target,
           warship_palmares_nvs_backend(), &ctx),
           WARSHIP_PALMARES_READ_ERROR);
    EQ(ctx.last_error, ESP_FAIL);
    fake.open_error = ESP_OK;

    /* Version/size/corruption is rejected without touching NVS. */
    fake.size = WARSHIP_PALMARES_WIRE_SIZE + 2u;
    const unsigned old_writes = fake.sets;
    STATUS(warship_palmares_open(&target,
           warship_palmares_nvs_backend(), &ctx),
           WARSHIP_PALMARES_CORRUPT);
    EQ(fake.sets, old_writes);
    fake.size = WARSHIP_PALMARES_WIRE_SIZE;
    fake.blob[8u] ^= 0x80u;
    STATUS(warship_palmares_open(&target,
           warship_palmares_nvs_backend(), &ctx),
           WARSHIP_PALMARES_CORRUPT);
    EQ(fake.sets, old_writes);
}
int main(void)
{
    test_lifecycle();
    printf("D08 NVS adapter host-stub PASS\n");
    return EXIT_SUCCESS;
}
