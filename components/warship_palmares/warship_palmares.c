#include "warship_palmares.h"

#include <limits.h>
#include <string.h>

#define PAL_MAGIC UINT32_C(0x50303831)
#define PAL_WIRE_MAGIC UINT32_C(0x57535031)
#define PAL_WIRE_VERSION UINT32_C(1)

static int initialized(const WarshipPalmares *p)
{
    return p != NULL && p->magic == PAL_MAGIC &&
           p->backend.load != NULL && p->backend.store != NULL;
}

static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint64_t get64(const uint8_t *p)
{
    return (uint64_t)get32(p) | ((uint64_t)get32(p + 4) << 32);
}
static void put32(uint8_t *p, uint32_t v)
{
    for (unsigned i = 0; i < 4u; ++i)
        p[i] = (uint8_t)(v >> (i * 8u));
}
static void put64(uint8_t *p, uint64_t v)
{
    put32(p, (uint32_t)v);
    put32(p + 4, (uint32_t)(v >> 32));
}
static uint32_t checksum(const uint8_t *bytes)
{
    uint32_t hash = UINT32_C(2166136261);
    for (unsigned i = 0u; i < 40u; ++i) {
        hash ^= bytes[i];
        hash *= UINT32_C(16777619);
    }
    return hash;
}
static int valid_event(uint32_t type)
{
    return type >= WARSHIP_LIFECYCLE_EVENT_VICTORY &&
           type <= WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT;
}
static int consistent(const WarshipPalmaresView *v,
                      uint64_t last_id, uint32_t type)
{
    const uint64_t sum = (uint64_t)v->victories + v->defeats + v->draws;
    if (sum != v->games) return 0;
    if (last_id == 0u)
        return type == WARSHIP_LIFECYCLE_EVENT_NONE &&
               v->games == 0u && v->voluntary_forfeits == 0u;
    if (!valid_event(type)) return 0;
    switch (type) {
    case WARSHIP_LIFECYCLE_EVENT_VICTORY: return v->victories != 0u;
    case WARSHIP_LIFECYCLE_EVENT_DEFEAT: return v->defeats != 0u;
    case WARSHIP_LIFECYCLE_EVENT_DRAW: return v->draws != 0u;
    default: return v->voluntary_forfeits != 0u;
    }
}

static void encode(const WarshipPalmares *p,
                   uint8_t bytes[WARSHIP_PALMARES_WIRE_SIZE])
{
    put32(bytes, PAL_WIRE_MAGIC);
    put32(bytes + 4, PAL_WIRE_VERSION);
    put32(bytes + 8, p->counters.games);
    put32(bytes + 12, p->counters.victories);
    put32(bytes + 16, p->counters.defeats);
    put32(bytes + 20, p->counters.draws);
    put32(bytes + 24, p->counters.voluntary_forfeits);
    put64(bytes + 28, p->last_id);
    put32(bytes + 36, p->last_event);
    put32(bytes + 40, checksum(bytes));
}

static WarshipPalmaresStatus decode(
    const uint8_t bytes[WARSHIP_PALMARES_WIRE_SIZE],
    WarshipPalmares *out)
{
    if (get32(bytes) != PAL_WIRE_MAGIC) return WARSHIP_PALMARES_CORRUPT;
    if (get32(bytes + 4) != PAL_WIRE_VERSION)
        return WARSHIP_PALMARES_INCOMPATIBLE_VERSION;
    if (get32(bytes + 40) != checksum(bytes))
        return WARSHIP_PALMARES_CORRUPT;
    WarshipPalmares p;
    memset(&p, 0, sizeof p);
    p.counters.games = get32(bytes + 8);
    p.counters.victories = get32(bytes + 12);
    p.counters.defeats = get32(bytes + 16);
    p.counters.draws = get32(bytes + 20);
    p.counters.voluntary_forfeits = get32(bytes + 24);
    p.last_id = get64(bytes + 28);
    p.last_event = get32(bytes + 36);
    if (!consistent(&p.counters, p.last_id, p.last_event))
        return WARSHIP_PALMARES_CORRUPT;
    out->counters = p.counters;
    out->last_id = p.last_id;
    out->last_event = p.last_event;
    return WARSHIP_PALMARES_APPLIED;
}

static WarshipPalmaresStatus load_snapshot(WarshipPalmares *p)
{
    uint8_t bytes[WARSHIP_PALMARES_WIRE_SIZE] = {0};
    size_t actual = 0u;
    const int status = p->backend.load(p->backend_context, bytes,
                                       sizeof bytes, &actual);
    if (status == 1) {
        memset(&p->counters, 0, sizeof p->counters);
        p->last_id = 0u;
        p->last_event = WARSHIP_LIFECYCLE_EVENT_NONE;
        return WARSHIP_PALMARES_APPLIED;
    }
    if (status != 0) return WARSHIP_PALMARES_READ_ERROR;
    if (actual != sizeof bytes) return WARSHIP_PALMARES_CORRUPT;
    return decode(bytes, p);
}

WarshipPalmaresStatus warship_palmares_open(
    WarshipPalmares *out, WarshipPalmaresBackend backend, void *context)
{
    if (out == NULL || backend.load == NULL || backend.store == NULL)
        return WARSHIP_PALMARES_BAD_ARGUMENT;
    WarshipPalmares candidate;
    memset(&candidate, 0, sizeof candidate);
    candidate.backend = backend;
    candidate.backend_context = context;
    const WarshipPalmaresStatus status = load_snapshot(&candidate);
    if (status != WARSHIP_PALMARES_APPLIED) return status;
    candidate.magic = PAL_MAGIC;
    *out = candidate;
    return WARSHIP_PALMARES_APPLIED;
}

WarshipPalmaresStatus warship_palmares_read(
    const WarshipPalmares *ledger, WarshipPalmaresView *out)
{
    if (!initialized(ledger) || out == NULL)
        return WARSHIP_PALMARES_BAD_ARGUMENT;
    if (ledger->uncertain != 0u) return WARSHIP_PALMARES_RELOAD_REQUIRED;
    *out = ledger->counters;
    return WARSHIP_PALMARES_APPLIED;
}

WarshipPalmaresStatus warship_palmares_reload(WarshipPalmares *ledger)
{
    if (!initialized(ledger)) return WARSHIP_PALMARES_BAD_ARGUMENT;
    WarshipPalmares fresh = *ledger;
    const WarshipPalmaresStatus status = load_snapshot(&fresh);
    if (status != WARSHIP_PALMARES_APPLIED) return status;
    fresh.uncertain = 0u;
    *ledger = fresh;
    return WARSHIP_PALMARES_APPLIED;
}

WarshipPalmaresStatus warship_palmares_apply(
    WarshipPalmares *ledger, uint64_t id, WarshipLifecycleEvent event)
{
    if (!initialized(ledger)) return WARSHIP_PALMARES_BAD_ARGUMENT;
    if (ledger->uncertain != 0u) return WARSHIP_PALMARES_RELOAD_REQUIRED;
    if (event == WARSHIP_LIFECYCLE_EVENT_NONE)
        return WARSHIP_PALMARES_NO_CHANGE;
    if (!valid_event((uint32_t)event) || id == 0u)
        return WARSHIP_PALMARES_BAD_EVENT;
    if (id == ledger->last_id)
        return (uint32_t)event == ledger->last_event ?
            WARSHIP_PALMARES_ALREADY_APPLIED :
            WARSHIP_PALMARES_CONFLICTING_ID;
    if (id < ledger->last_id) return WARSHIP_PALMARES_STALE_ID;
    WarshipPalmares candidate = *ledger;
    WarshipPalmaresView *v = &candidate.counters;
    if (event == WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT) {
        if (v->voluntary_forfeits == UINT32_MAX)
            return WARSHIP_PALMARES_OVERFLOW;
        ++v->voluntary_forfeits;
    } else {
        uint32_t *category = event == WARSHIP_LIFECYCLE_EVENT_VICTORY ?
            &v->victories : event == WARSHIP_LIFECYCLE_EVENT_DEFEAT ?
            &v->defeats : &v->draws;
        if (v->games == UINT32_MAX || *category == UINT32_MAX)
            return WARSHIP_PALMARES_OVERFLOW;
        ++v->games;
        ++*category;
    }
    candidate.last_id = id;
    candidate.last_event = (uint32_t)event;
    if (!consistent(v, id, (uint32_t)event))
        return WARSHIP_PALMARES_CORRUPT;
    uint8_t bytes[WARSHIP_PALMARES_WIRE_SIZE];
    encode(&candidate, bytes);
    if (ledger->backend.store(ledger->backend_context,
                              bytes, sizeof bytes) != 0) {
        /* Commit may have succeeded despite the error. Never apply other
         * IDs using this stale cache. First reload the physical backend. */
        ledger->uncertain = 1u;
        return WARSHIP_PALMARES_WRITE_UNCERTAIN;
    }
    *ledger = candidate;
    return WARSHIP_PALMARES_APPLIED;
}
