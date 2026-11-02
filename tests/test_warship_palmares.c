#include "warship_palmares.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
    exit(EXIT_FAILURE); \
} } while (0)
#define EQ(a,b) CHECK((a) == (b))
#define STATUS(expr, expected) EQ((expr), (expected))
#define APPLIED(expr) STATUS((expr), WARSHIP_PALMARES_APPLIED)

typedef struct {
    uint8_t bytes[WARSHIP_PALMARES_WIRE_SIZE];
    size_t length;
    unsigned loads, stores;
    int exists, read_fail, write_before_fail, write_after_fail;
} FakeNvs;

static int fake_load(void *ctx, uint8_t *out, size_t capacity, size_t *actual)
{
    FakeNvs *s = (FakeNvs *)ctx;
    ++s->loads;
    if (s->read_fail) return -1;
    if (!s->exists) return 1;
    *actual = s->length;
    const size_t size = s->length > capacity ? capacity : s->length;
    memcpy(out, s->bytes, size);
    return 0;
}
static int fake_store(void *ctx, const uint8_t *in, size_t size)
{
    FakeNvs *s = (FakeNvs *)ctx;
    ++s->stores;
    if (s->write_before_fail) return -1;
    CHECK(size == WARSHIP_PALMARES_WIRE_SIZE);
    memcpy(s->bytes, in, size);
    s->length = size;
    s->exists = 1;
    return s->write_after_fail ? -1 : 0;
}
static const WarshipPalmaresBackend backend = {fake_load, fake_store};

static void same(const void *a, const void *b, size_t size)
{
    CHECK(memcmp(a, b, size) == 0);
}
static WarshipPalmares open_storage(FakeNvs *nvs)
{
    WarshipPalmares p;
    memset(&p, 0x9a, sizeof p);
    APPLIED(warship_palmares_open(&p, backend, nvs));
    return p;
}
static WarshipPalmaresView read_view(WarshipPalmares *p)
{
    WarshipPalmaresView out;
    memset(&out, 0x44, sizeof out);
    APPLIED(warship_palmares_read(p, &out));
    CHECK((uint64_t)out.victories + out.defeats + out.draws == out.games);
    return out;
}
static void counts(WarshipPalmares *p, uint32_t games, uint32_t win,
                   uint32_t loss, uint32_t draw, uint32_t forfeits)
{
    WarshipPalmaresView v = read_view(p);
    EQ(v.games, games);
    EQ(v.victories, win);
    EQ(v.defeats, loss);
    EQ(v.draws, draw);
    EQ(v.voluntary_forfeits, forfeits);
}
static void resnapshot(FakeNvs *s)
{
    /* Independent test-side checksum for crafting controlled bad/capacity
     * records; does not call the production decoder or encoder. */
    uint32_t hash = UINT32_C(2166136261);
    for (unsigned i = 0u; i < 40u; ++i) {
        hash ^= s->bytes[i];
        hash *= UINT32_C(16777619);
    }
    for (unsigned i = 0u; i < 4u; ++i)
        s->bytes[40u + i] = (uint8_t)(hash >> (8u * i));
}
static void put32(FakeNvs *s, unsigned offset, uint32_t value)
{
    CHECK(offset < 40u && offset % 4u == 0u);
    for (unsigned i = 0u; i < 4u; ++i)
        s->bytes[offset + i] = (uint8_t)(value >> (8u * i));
}
static void put64(FakeNvs *s, unsigned offset, uint64_t value)
{
    put32(s, offset, (uint32_t)value);
    put32(s, offset + 4u, (uint32_t)(value >> 32u));
}

static void tests_invalid(void)
{
    FakeNvs storage = {0};
    WarshipPalmares ledger;
    memset(&ledger, 0xa7, sizeof ledger);
    WarshipPalmares prior = ledger;
    WarshipPalmaresBackend null_backend = {0};
    STATUS(warship_palmares_open(NULL, backend, &storage),
           WARSHIP_PALMARES_BAD_ARGUMENT);
    STATUS(warship_palmares_open(&ledger, null_backend, &storage),
           WARSHIP_PALMARES_BAD_ARGUMENT);
    same(&ledger, &prior, sizeof ledger);
    WarshipPalmaresView out;
    memset(&out, 0x73, sizeof out);
    WarshipPalmaresView out_prior = out;
    STATUS(warship_palmares_read(&ledger, &out),
           WARSHIP_PALMARES_BAD_ARGUMENT);
    STATUS(warship_palmares_apply(&ledger, 1u,
           WARSHIP_LIFECYCLE_EVENT_VICTORY),
           WARSHIP_PALMARES_BAD_ARGUMENT);
    STATUS(warship_palmares_reload(&ledger),
           WARSHIP_PALMARES_BAD_ARGUMENT);
    same(&ledger, &prior, sizeof ledger);
    same(&out, &out_prior, sizeof out);
    ledger = open_storage(&storage);
    STATUS(warship_palmares_read(&ledger, NULL),
           WARSHIP_PALMARES_BAD_ARGUMENT);
    STATUS(warship_palmares_apply(&ledger, 0u,
           WARSHIP_LIFECYCLE_EVENT_DRAW),
           WARSHIP_PALMARES_BAD_EVENT);
    STATUS(warship_palmares_apply(&ledger, 5u,
           (WarshipLifecycleEvent)255),
           WARSHIP_PALMARES_BAD_EVENT);
    EQ(storage.stores, 0u);
    counts(&ledger, 0u, 0u, 0u, 0u, 0u);
}

static void tests_nominal_and_reboot(void)
{
    FakeNvs storage = {0};
    WarshipPalmares p = open_storage(&storage);
    EQ(storage.loads, 1u);
    EQ(storage.stores, 0u);
    counts(&p, 0u, 0u, 0u, 0u, 0u);
    const WarshipPalmares old = p;
    STATUS(warship_palmares_apply(&p, 0u,
           WARSHIP_LIFECYCLE_EVENT_NONE), WARSHIP_PALMARES_NO_CHANGE);
    STATUS(warship_palmares_apply(&p, 123u,
           WARSHIP_LIFECYCLE_EVENT_NONE), WARSHIP_PALMARES_NO_CHANGE);
    same(&p, &old, sizeof p);
    EQ(storage.stores, 0u);
    APPLIED(warship_palmares_apply(&p, 10u,
            WARSHIP_LIFECYCLE_EVENT_VICTORY));
    counts(&p, 1u, 1u, 0u, 0u, 0u);
    EQ(storage.stores, 1u);
    const WarshipPalmares after_win = p;
    const uint8_t disk_win[WARSHIP_PALMARES_WIRE_SIZE] = {0};
    (void)disk_win;
    STATUS(warship_palmares_apply(&p, 10u,
           WARSHIP_LIFECYCLE_EVENT_VICTORY),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    STATUS(warship_palmares_apply(&p, 10u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_CONFLICTING_ID);
    STATUS(warship_palmares_apply(&p, 9u,
           WARSHIP_LIFECYCLE_EVENT_VICTORY),
           WARSHIP_PALMARES_STALE_ID);
    same(&p, &after_win, sizeof p);
    EQ(storage.stores, 1u);
    p = open_storage(&storage);
    counts(&p, 1u, 1u, 0u, 0u, 0u);
    STATUS(warship_palmares_apply(&p, 10u,
           WARSHIP_LIFECYCLE_EVENT_VICTORY),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    APPLIED(warship_palmares_apply(&p, 11u,
            WARSHIP_LIFECYCLE_EVENT_DEFEAT));
    APPLIED(warship_palmares_apply(&p, 12u,
            WARSHIP_LIFECYCLE_EVENT_DRAW));
    APPLIED(warship_palmares_apply(&p, 19u,
            WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT));
    counts(&p, 3u, 1u, 1u, 1u, 1u);
    EQ(storage.stores, 4u);
    WarshipPalmares again = open_storage(&storage);
    counts(&again, 3u, 1u, 1u, 1u, 1u);
    STATUS(warship_palmares_apply(&again, 12u,
           WARSHIP_LIFECYCLE_EVENT_DRAW),
           WARSHIP_PALMARES_STALE_ID);
    STATUS(warship_palmares_apply(&again, 19u,
           WARSHIP_LIFECYCLE_EVENT_VICTORY),
           WARSHIP_PALMARES_CONFLICTING_ID);
    STATUS(warship_palmares_apply(&again, 19u,
           WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    EQ(storage.stores, 4u);
    /* A new non-event lifecycle observation must never count anything. */
    STATUS(warship_palmares_apply(&again, UINT64_MAX,
           WARSHIP_LIFECYCLE_EVENT_NONE),
           WARSHIP_PALMARES_NO_CHANGE);
    counts(&again, 3u, 1u, 1u, 1u, 1u);
}

static void tests_commit_failures(void)
{
    FakeNvs storage = {0};
    WarshipPalmares p = open_storage(&storage);
    storage.write_before_fail = 1;
    STATUS(warship_palmares_apply(&p, 1u,
           WARSHIP_LIFECYCLE_EVENT_VICTORY),
           WARSHIP_PALMARES_WRITE_UNCERTAIN);
    EQ(storage.stores, 1u);
    EQ(storage.exists, 0);
    WarshipPalmaresView out;
    memset(&out, 0xa5, sizeof out);
    WarshipPalmaresView before = out;
    STATUS(warship_palmares_read(&p, &out),
           WARSHIP_PALMARES_RELOAD_REQUIRED);
    same(&before, &out, sizeof out);
    STATUS(warship_palmares_apply(&p, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_RELOAD_REQUIRED);
    EQ(storage.stores, 1u);
    storage.read_fail = 1;
    const WarshipPalmares uncertain = p;
    STATUS(warship_palmares_reload(&p), WARSHIP_PALMARES_READ_ERROR);
    same(&p, &uncertain, sizeof p);
    storage.read_fail = 0;
    APPLIED(warship_palmares_reload(&p));
    counts(&p, 0u, 0u, 0u, 0u, 0u);
    storage.write_before_fail = 0;
    APPLIED(warship_palmares_apply(&p, 1u,
            WARSHIP_LIFECYCLE_EVENT_VICTORY));
    counts(&p, 1u, 1u, 0u, 0u, 0u);
    storage.write_after_fail = 1;
    STATUS(warship_palmares_apply(&p, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_WRITE_UNCERTAIN);
    /* COMMIT was actually durable, but API has no right to assume it. */
    EQ(storage.stores, 3u);
    STATUS(warship_palmares_read(&p, &out),
           WARSHIP_PALMARES_RELOAD_REQUIRED);
    storage.write_after_fail = 0;
    WarshipPalmares rebooted = open_storage(&storage);
    counts(&rebooted, 2u, 1u, 1u, 0u, 0u);
    STATUS(warship_palmares_apply(&rebooted, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    APPLIED(warship_palmares_reload(&p));
    counts(&p, 2u, 1u, 1u, 0u, 0u);
    STATUS(warship_palmares_apply(&p, 2u,
           WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    EQ(storage.stores, 3u);
    APPLIED(warship_palmares_apply(&p, 3u,
            WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT));
    counts(&p, 2u, 1u, 1u, 0u, 1u);
}

static void tests_corrupt_and_versions(void)
{
    FakeNvs storage = {0};
    WarshipPalmares p = open_storage(&storage);
    APPLIED(warship_palmares_apply(&p, 20u,
            WARSHIP_LIFECYCLE_EVENT_DRAW));
    FakeNvs saved = storage;
    WarshipPalmares destination;
    memset(&destination, 0x33, sizeof destination);
    const WarshipPalmares old = destination;
    storage.read_fail = 1;
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_READ_ERROR);
    same(&destination, &old, sizeof destination);
    storage = saved;
    storage.length = 30u;
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_CORRUPT);
    storage.length = WARSHIP_PALMARES_WIRE_SIZE + 10u;
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_CORRUPT);
    same(&destination, &old, sizeof destination);
    storage = saved;
    storage.bytes[8u] ^= 0x55u;
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_CORRUPT);
    storage = saved;
    put32(&storage, 0u, 0u);
    resnapshot(&storage);
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_CORRUPT);
    storage = saved;
    put32(&storage, 4u, 2u);
    resnapshot(&storage);
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_INCOMPATIBLE_VERSION);
    storage = saved;
    put32(&storage, 8u, 2u); /* inconsistent games != draw sum */
    resnapshot(&storage);
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_CORRUPT);
    storage = saved;
    put32(&storage, 36u, 255u); /* unknown last event */
    resnapshot(&storage);
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_CORRUPT);
    storage = saved;
    put64(&storage, 28u, 0u); /* count nonzero but no marker */
    put32(&storage, 36u, WARSHIP_LIFECYCLE_EVENT_NONE);
    resnapshot(&storage);
    STATUS(warship_palmares_open(&destination, backend, &storage),
           WARSHIP_PALMARES_CORRUPT);
    same(&destination, &old, sizeof destination);
    EQ(storage.stores, saved.stores); /* Never wipe/repair bad NVS. */
    storage = saved;
    APPLIED(warship_palmares_open(&destination, backend, &storage));
    counts(&destination, 1u, 0u, 0u, 1u, 0u);
}

static void tests_overflow_and_large_ids(void)
{
    FakeNvs storage = {0};
    WarshipPalmares p = open_storage(&storage);
    APPLIED(warship_palmares_apply(&p, 1u,
            WARSHIP_LIFECYCLE_EVENT_VICTORY));
    /* Forge legitimate edge records, re-checksum, then reopen via decoder.
     * This exercises overflow without 2^32 actual flash writes. */
    put32(&storage, 8u, UINT32_MAX);
    put32(&storage, 12u, UINT32_MAX);
    resnapshot(&storage);
    p = open_storage(&storage);
    const WarshipPalmares prior = p;
    STATUS(warship_palmares_apply(&p, 2u,
           WARSHIP_LIFECYCLE_EVENT_DRAW), WARSHIP_PALMARES_OVERFLOW);
    same(&p, &prior, sizeof p);
    EQ(storage.stores, 1u);
    storage = (FakeNvs){0};
    p = open_storage(&storage);
    APPLIED(warship_palmares_apply(&p, 1u,
            WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT));
    put32(&storage, 24u, UINT32_MAX);
    resnapshot(&storage);
    p = open_storage(&storage);
    STATUS(warship_palmares_apply(&p, 3u,
           WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT),
           WARSHIP_PALMARES_OVERFLOW);
    counts(&p, 0u, 0u, 0u, 0u, UINT32_MAX);
    /* ID limit is explicitly accepted and then no greater IDs exist. */
    storage = (FakeNvs){0};
    p = open_storage(&storage);
    APPLIED(warship_palmares_apply(&p, UINT64_MAX,
            WARSHIP_LIFECYCLE_EVENT_DRAW));
    p = open_storage(&storage);
    STATUS(warship_palmares_apply(&p, UINT64_MAX,
           WARSHIP_LIFECYCLE_EVENT_DRAW),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    STATUS(warship_palmares_apply(&p, UINT64_MAX - 1u,
           WARSHIP_LIFECYCLE_EVENT_DRAW),
           WARSHIP_PALMARES_STALE_ID);
}

static int rng(void *unused, uint32_t *out)
{
    (void)unused;
    *out = 0u;
    return 0;
}
static void tests_d07_event_contract(void)
{
    FakeNvs storage = {0};
    WarshipPalmares p = open_storage(&storage);
    WarshipLifecycle game;
    CHECK(warship_lifecycle_init(&game) == WARSHIP_LIFECYCLE_OK);
    CHECK(warship_lifecycle_start(&game, rng, NULL, rng, NULL) ==
          WARSHIP_LIFECYCLE_OK);
    /* PRET is the only engagement, and leaving first deployment creates
     * no D07 event. The PALMARÈS reducer is never called for such UI. */
    CHECK(warship_lifecycle_leave_deploy(&game) ==
          WARSHIP_LIFECYCLE_OK);
    EQ(storage.stores, 0u);
    CHECK(warship_lifecycle_start(&game, rng, NULL, rng, NULL) ==
          WARSHIP_LIFECYCLE_OK);
    CHECK(warship_lifecycle_ready(&game) == WARSHIP_LIFECYCLE_OK);
    CHECK(warship_lifecycle_request_abandon(&game) ==
          WARSHIP_LIFECYCLE_OK);
    WarshipLifecycleEvent event = WARSHIP_LIFECYCLE_EVENT_DRAW;
    CHECK(warship_lifecycle_decide_abandon(&game, 0u, &event) ==
          WARSHIP_LIFECYCLE_OK);
    EQ(event, WARSHIP_LIFECYCLE_EVENT_NONE);
    STATUS(warship_palmares_apply(&p, 500u, event),
           WARSHIP_PALMARES_NO_CHANGE);
    CHECK(warship_lifecycle_request_abandon(&game) ==
          WARSHIP_LIFECYCLE_OK);
    CHECK(warship_lifecycle_decide_abandon(&game, 1u, &event) ==
          WARSHIP_LIFECYCLE_OK);
    EQ(event, WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT);
    APPLIED(warship_palmares_apply(&p, 500u, event));
    counts(&p, 0u, 0u, 0u, 0u, 1u);
    STATUS(warship_palmares_apply(&p, 500u, event),
           WARSHIP_PALMARES_ALREADY_APPLIED);
    EQ(storage.stores, 1u);
}
int main(void)
{
    tests_invalid();
    tests_nominal_and_reboot();
    tests_commit_failures();
    tests_corrupt_and_versions();
    tests_overflow_and_large_ids();
    tests_d07_event_contract();
    printf("D08 PASS snapshot=%u sizeof(ledger)=%zu sizeof(view)=%zu\n",
           (unsigned)WARSHIP_PALMARES_WIRE_SIZE,
           sizeof(WarshipPalmares), sizeof(WarshipPalmaresView));
    return EXIT_SUCCESS;
}
